#include "SM_TwoThres.hh"
#include "debug/EnergyMgmt.hh"

SM_TwoThres::SM_TwoThres(const Params *p) 
    : BaseEnergySM(p),
    state(SM_TwoThres::State::STATE_POWEROFF),
    thres_1_to_off(p->thres_1_to_off),
    thres_off_to_1(p->thres_off_to_1)
{
    // when the system cannot consume energy
    // TO_TEST: BaseEnergySM::
    energy_consume_lower_bound = thres_1_to_off;
}

void SM_TwoThres::init()
{
    EnergyMsg msg;
    msg.val = 0;
    state = State::STATE_POWEROFF;
    msg.type = MsgType::MSG_POWEROFF;
    broadcastMsg(msg);
}

void SM_TwoThres::update(double _energy)
{
    EnergyMsg msg;
    msg.val = 0;

    // State Machine Impl.
    if (state == State::STATE_POWEROFF && _energy >= thres_off_to_1)
    {
        DPRINTF(EnergyMgmt, "State change: STATE_POWEROFF->STATE_POWERON, energy=%lf, thres=%lf.\n", _energy, thres_off_to_1);
        state = State::STATE_POWERON;
        msg.type = MsgType::MSG_POWERON;
        broadcastMsg(msg);
    } 

    else if (state == State::STATE_POWERON && _energy <= thres_1_to_off)
    {
        DPRINTF(EnergyMgmt, "State change: STATE_POWERON->STATE_POWEROFF, energy=%lf, thres=%lf.\n", _energy, thres_1_to_off);
        state = State::STATE_POWERON;
        msg.type = MsgType::MSG_POWERON;
        broadcastMsg(msg);
    }
}

SM_TwoThres *
SM_TwoThresParams::create()
{
    return new SM_TwoThres(this);
}