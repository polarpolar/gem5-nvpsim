//
// Created by lf-z on 12/28/16.
//

#ifndef GEM5_ENGY_HH
#define GEM5_ENGY_HH

#include "engy/energy_harvester.hh"
#include "engy/state_machine.hh"
#include "sim/sim_object.hh"
#include "params/EnergyMgmt.hh"

class EnergyMgmt : public SimObject
{
public:
    typedef EnergyMgmtParams Params;
    const Params *params() const
    {
        return reinterpret_cast<const Params *>(_params);
    }
    EnergyMgmt(const Params *p);
    virtual ~EnergyMgmt();
    virtual void init();
    // Harvest energy if val < 0
    virtual int consumeEnergy(char *consumer, double val);
    void broadcastMsg();
    int broadcastMsgAsEvent(const EnergyMsg &msg);
    int handleMsg(const EnergyMsg &msg);

		double system_leakage;
		double energy_profile_mult;
protected:
    Tick time_unit;
    double energy_remained;
    /* msg_togo is changed into a queue to prevent bugs in case that multiple state changes occurs in one tick. */
    std::vector<EnergyMsg> msg_togo;
    EventWrapper<EnergyMgmt, &EnergyMgmt::broadcastMsg> event_msg;
    std::vector<double> energy_harvest_data;
    void energyHarvest();
    EventWrapper<EnergyMgmt, &EnergyMgmt::energyHarvest> event_energy_harvest;
    BaseEnergySM *state_machine;
    BaseHarvester *harvest_module;
    double capacity;

    // for all SMs, the consuming msg must be 0.
    enum MsgType
    {
        CONSUME_ENERGY = 0
    };

private:
    std::vector<double> readEnergyProfile();
    std::string _path_energy_profile;

};

#endif //GEM5_ENGY_HH
