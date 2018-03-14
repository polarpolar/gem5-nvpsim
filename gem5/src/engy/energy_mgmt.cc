//
// Created by lf-z on 12/28/16.
//
#include <fstream>
#include <math.h>
#include <string.h>
#include "sim/eventq.hh"
#include "engy/energy_mgmt.hh"
#include "engy/state_machine.hh"
#include "engy/energy_harvester.hh"
#include "debug/EnergyMgmt.hh"
#include "debug/VirtualDevice.hh"

extern bool DFS_LRY_poweron_dirty_patch;

EnergyMgmt::EnergyMgmt(const Params *p)
        : SimObject(p),
        	system_leakage(p->system_leakage),
        	energy_profile_mult(p->energy_profile_mult),
          time_unit(p->energy_time_unit),
          energy_remained(0),
          event_msg(this, false, Event::Energy_Pri),
          event_energy_harvest(this, false, Event::Energy_Pri),
          state_machine(p->state_machine),
          harvest_module(p->harvest_module),
          capacity(p->capacity),
          _path_energy_profile(p->path_energy_profile)
{
    msg_togo.resize(0);
}

EnergyMgmt::~EnergyMgmt() {}

void EnergyMgmt::init()
{
    /* Read energy profile. */
    energy_harvest_data = readEnergyProfile();
    /* Reset energy remained to 0. */
    energy_remained = 0;
    /* Set mgmt pointer in state machine. */
    if (state_machine) {
        state_machine->mgmt = this;
    }

    DPRINTF(EnergyMgmt, "[EngyMgmt] Energy Management module initialized!\n");
    DPRINTF(EnergyMgmt, "[EngyMgmt] Energy profile: %s (Time unit: %d ticks)\n",
            _path_energy_profile.c_str(), time_unit);
    DPRINTF(EnergyMgmt, "[EngyMgmt] The capacity is %lf.\n", capacity);

    /* Start to harvest energy */
    energyHarvest();
}

// Handling the energy consumption as well as energy harvesting
int 
EnergyMgmt::consumeEnergy(char *sender, double val)
{
    /* Consume energy if val > 0, and harvest energy if val < 0 */
    double cons_unit, harv_unit;
    double cap_volt_upper_bound = 5;
    double lower_bound = state_machine->energy_consume_lower_bound; // nJ
    double upper_bound = 0.5 * capacity * pow(cap_volt_upper_bound, 2) * pow(10, 3); // nJ

    // Energy Consumption, if val > 0
    if (val > 0) {
        energy_remained -= val;
        cons_unit = val;
        // The energy remained has a lower bound. When the lower bound is meet, the system need to power off. Energy > 0 / retention threshold.
        if (energy_remained < lower_bound) {
            cons_unit -= (lower_bound - energy_remained);
            energy_remained = lower_bound;
        }
        if (strcmp(sender, "VDEV-2")==0 && curTick()%10000 == 0)
            DPRINTF(EnergyMgmt, "Energy %lf is consumed by %s. Energy remained: %lf\n", cons_unit, sender, energy_remained);
    } 

    // Energy Harvesting, if val < 0
    else {
        // Todo: remove the leakage to consumptions.
        // energy leakage!
        if (DFS_LRY_poweron_dirty_patch)
        {
            energy_remained -= system_leakage;
            //DPRINTF(EnergyMgmt, "[EngyMgmt] Leakage energy is %lf. Energy remained: %lf\n", system_leakage, energy_remained);
        }

        // energy harvesting        
        val *= energy_profile_mult; // adjust the supply
        harv_unit = -val;
        energy_remained = harvest_module->energy_harvest(-val, energy_remained);

        // The energy storage has a upper bound
        if (energy_remained > upper_bound) {
            harv_unit -= (energy_remained - upper_bound);
            energy_remained = upper_bound;
        }
        // consider of leakage, the system may fall below lower bound.
        else if (energy_remained < lower_bound) {
            energy_remained = lower_bound;
        }
        DPRINTF(EnergyMgmt, "[EngyMgmt] Energy %lf is harvested. Energy remained: %lf\n", harv_unit, energy_remained);
    }
    
    // judge if energy_remained triggers state_machine changes
    state_machine->update(energy_remained);

    return 1;
}

// Broadcasr energy messages via master energy port
void 
EnergyMgmt::broadcastMsg()
{
    /* Broadcast the first message in the msg queue. */
    _meport.broadcastMsg(msg_togo[0]);
    /* Delete the message we broadcast. */
    msg_togo.erase(msg_togo.begin());
    /* If there are still messages, schedule a new event. */
    if (!msg_togo.empty())
        schedule(event_msg, curTick());
}

// Todo: find out the usage of this function
int EnergyMgmt::broadcastMsgAsEvent(const EnergyMsg &msg)
{
    msg_togo.push_back(msg);
    /* Trigger first msg in the current tick. */
    if (msg_togo.size() == 1)
        schedule(event_msg, curTick());
    return 1;
}

// The energy manager is not only the message sender but also a receiver. All the messages processed here. The energy manager handles the energy consuming/harvesting events, while for other messages, this function will make a statement of the messages.
int 
EnergyMgmt::handleMsg(const EnergyMsg &msg)
{
    // Statement of the current message (except consumes)
    if (msg.type != CONSUME_ENERGY)
    {
        DPRINTF(EnergyMgmt, "HandleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
    }

    else if (msg.type == CONSUME_ENERGY)
    {
        return consumeEnergy(msg.sender, msg.val);
    }

    return 0;
}

std::vector<double> EnergyMgmt::readEnergyProfile()
{
    std::vector<double> data;
    data.resize(0);
    /* Return empty vector is no energy profile is provided. */
    if (_path_energy_profile == "")
        return data;

    double temp;
    std::ifstream fin;
    fin.open(_path_energy_profile.c_str());
    /* Read energy profile and store the data into vector. */
    while (fin>>temp) {
        data.push_back(temp);
    }
    /* Reverse the energy harvest queue so that the first energy unit pops first */
    reverse(data.begin(), data.end());
    fin.close();
    return data;
}

// Energy harvest function provides an harvest event, where an Energy_Consume_Msg with negative value is generated. One by one, this event will trigger the next harvest event.
void 
EnergyMgmt::energyHarvest()
{
    // read the energy supply data from the energy profile
    if (energy_harvest_data.empty())
        return;

    double energy_val = energy_harvest_data.back();
    char dev_name[100];
    strcpy(dev_name, "harvester");
    consumeEnergy(dev_name, -energy_val);
    energy_harvest_data.pop_back();

    /* Trigger the next harvest function. */
    schedule(event_energy_harvest, curTick() + time_unit);

    // DPRINTF(EnergyMgmt, "Energy %lf harvested.\n", energy_val);
}

EnergyMgmt *
EnergyMgmtParams::create()
{
    return new EnergyMgmt(this);
}
