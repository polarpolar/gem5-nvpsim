//
// Created by lf-z on 3/13/17.
// Update by tongda on 3/14/18.
//

#include "engy/state_machine.hh"
#include "debug/EnergyMgmt.hh"
#include <fstream>

/******* BaseEnergySM *******/
BaseEnergySM::BaseEnergySM(const Params *p) : SimObject(p), mgmt(NULL)
{
	energy_consume_lower_bound = 0;
}

void 
BaseEnergySM::broadcastMsg(const EnergyMsg &msg)
{
	mgmt->broadcastMsgAsEvent(msg);
}

/******* SimpleEnergySM *******/
SimpleEnergySM::SimpleEnergySM(const Params *p) : 
	BaseEnergySM(p), 
	state(SimpleEnergySM::State::STATE_POWER_OFF),
	thres_1_to_off(p->thres_1_to_off),
	thres_off_to_1(p->thres_off_to_1)
{
	// when the system cannot consume energy
	energy_consume_lower_bound = thres_1_to_off;
}

void 
SimpleEnergySM::init()
{
	EnergyMsg msg;
	msg.val = 0;
	state = State::STATE_POWER_OFF;
	msg.type = MsgType::POWER_OFF;
	broadcastMsg(msg);

	//
	std::ofstream fout;
	fout.open("m5out/power_failure", std::ios::app);
	assert(fout);
	fout << outage_times << std::endl;
	fout.close();
}

void SimpleEnergySM::update(double _energy)
{
	EnergyMsg msg;
	msg.val = 0;

	// power failure
	if (state == STATE_POWER_ON && _energy <= thres_1_to_off) 
	{
		state = State::STATE_POWER_OFF;
		msg.type = MsgType::POWER_OFF;
		DPRINTF(EnergyMgmt, "[SimpleEngySM] State change: POWER_ON->POWER_OFF, energy=%lf, thres=%lf.\n", _energy, thres_1_to_off);

		// Calculate Power failure times
		outage_times++;
		std::ofstream fout;
		fout.open("m5out/power_failure", std::ios::app);
		assert(fout);
		fout << outage_times << std::endl;
		fout.close();

		broadcastMsg(msg);
	} 

	// power recovery
	else if (state == State::STATE_POWER_OFF && _energy >= thres_off_to_1) 
	{
		state = State::STATE_POWER_ON;
		msg.type = MsgType::POWER_ON;
		DPRINTF(EnergyMgmt, "[SimpleEngySM] State change: POWER_OFF->POWER_ON, energy=%lf, thres=%lf.\n", _energy, thres_off_to_1);
		broadcastMsg(msg);
	}
}

BaseEnergySM *
BaseEnergySMParams::create()
{
	return new BaseEnergySM(this);
}

SimpleEnergySM *
SimpleEnergySMParams::create()
{
	return new SimpleEnergySM(this);
}
