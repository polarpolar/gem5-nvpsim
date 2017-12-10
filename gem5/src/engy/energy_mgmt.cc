//
// Created by lf-z on 12/28/16.
//
#include <fstream>
#include <math.h>
#include "engy/energy_mgmt.hh"
#include "engy/state_machine.hh"
#include "engy/harvest.hh"
#include "debug/EnergyMgmt.hh"
#include "debug/VirtualDevice.hh"
#include "sim/eventq.hh"
#include "engy/DFS_LRY.hh"

//Õâ¸öÈ«¾Ö±äÁ¿ÓÃÓÚÍ¨Öªenergy_mgmtÏÖÔÚÏµÍ³µÄ×´Ì¬
//²»ÖªµÀÎªÉ¶energy_mgmtÊÕ²»µ½msg£¬ËùÒÔÖ»ÄÜ½«¾ÍÕâÃ´¸ãÁË
//Õâ¸ö±äÁ¿µÄ±¾ÌåÊÇÔÚDFS_LRY.ccÖÐ¶¨ÒåµÄ
extern bool DFS_LRY_poweron_dirty_patch;

EnergyMgmt::EnergyMgmt(const Params *p)
        : SimObject(p),
        	energy_consumed_per_harvest(p->energy_consumed_per_harvest),
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

EnergyMgmt::~EnergyMgmt()
{

}

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

    /* Trigger first energy harvest event here */
    energyHarvest();
}

int EnergyMgmt::consumeEnergy(double val)
{
    /* Todo: Pass the module which consumed the energy to this function. (Or DPRINTF in the module which consumes energy) */
    /* Consume energy if val > 0, and harvest energy if val < 0 */
    // Edit by wtd on 11/13/17: Add the upper/lower bound of capacity: capacity, [cap_volt_lower_bound, cap_volt_lower_bound]
    double cons_unit, harv_unit;
    double cap_volt_lower_bound = 0;
    double cap_volt_upper_bound = 5;
    double lower_bound = 0.5 * capacity * pow(cap_volt_lower_bound, 2) * pow(10, 3); // nJ
    double upper_bound = 0.5 * capacity * pow(cap_volt_upper_bound, 2) * pow(10, 3); // nJ

    // Energy Consumption, if val > 0
    if (val > 0) {
        energy_remained -= val;
        cons_unit = val;
        // The energy remained has a lower bound. When the lower bound is meet, the system need to power off. Energy > 0.
        if (energy_remained < lower_bound) {
            cons_unit -= (lower_bound - energy_remained);
            energy_remained = lower_bound;
        }
        /**REMOVE**/
        //DPRINTF(EnergyMgmt, "[EngyMgmt] Energy %lf is consumed by xxx. Energy remained: %lf\n", cons_unit, energy_remained);
        /*if (cons_unit) {
            DPRINTF(EnergyMgmt, "[EngyMgmt] Energy Storage Meets Lower Bound! capacity: %lf, energy: %lf\n", capacity, energy_remained);
        }*/
    } 

    // Energy Harvesting, if val > 0
    else {
        // This is energy harvesting
        val *= energy_profile_mult;
        energy_remained = harvest_module->energy_harvest(-val, energy_remained);
        harv_unit = -val;
        if (DFS_LRY_poweron_dirty_patch)
        {
            /**REMOVE**/
            //energy leakage!
            //DPRINTF(EnergyMgmt, "Leakage: %lf\n", energy_consumed_per_harvest);
            energy_remained -= energy_consumed_per_harvest;
        }

        // The energy storage has a upper bound
        if (energy_remained > upper_bound) {
            harv_unit -= (energy_remained - upper_bound);
            energy_remained = upper_bound;
        }
        // consider of leakage, the system may still lose energy after harvesting.
        else if (energy_remained < lower_bound) {
            energy_remained = lower_bound;
        }
        /**REMOVED**/
        //DPRINTF(EnergyMgmt, "[EngyMgmt] Energy %lf is harvested. Energy remained: %lf\n", harv_unit, energy_remained);
        /*if (harv_unit) {
            DPRINTF(EnergyMgmt, "[EngyMgmt] Energy Storage Meets Upper Bound! capacity: %lf, energy: %lf\n", capacity, upper_bound);
        }*/
    }
    
    DPRINTF(EnergyMgmt, "[EngyMgmt] Energy Remained: %lf\n", energy_remained);
    state_machine->update(energy_remained);

    return 1;
}

void EnergyMgmt::broadcastMsg()
{
    /* Broadcast the first message in the msg queue. */
    _meport.broadcastMsg(msg_togo[0]);
    /* Delete the message we broadcast. */
    msg_togo.erase(msg_togo.begin());
    /* If there are still messages, schedule a new event. */
    if (!msg_togo.empty())
        schedule(event_msg, curTick());
}

int EnergyMgmt::broadcastMsgAsEvent(const EnergyMsg &msg)
{
    msg_togo.push_back(msg);
    /* Trigger first msg in the current tick. */
    if (msg_togo.size() == 1)
        schedule(event_msg, curTick());
    return 1;
}

int EnergyMgmt::handleMsg(const EnergyMsg &msg)
{
    /**REMOVE**/
    if (msg.type != 0) // msg.type = 0 means consume.
        DPRINTF(EnergyMgmt, "[EnergyMgmt] handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
    /* msg type should be 0 here, for 0 represents energy consuming, */
    /* and EnergyMgmt module can only handle energy consuming msg*/
    if (msg.type != DFS_LRY::MsgType::CONSUMEENERGY)
        return 0;

    return consumeEnergy(msg.val);
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

void EnergyMgmt::energyHarvest()
{
    /* Add harvested energy into capacity. */
    if (energy_harvest_data.empty())
        return;

    double energy_val = energy_harvest_data.back();
    consumeEnergy(-energy_val);
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
