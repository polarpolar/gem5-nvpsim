//
// Created by lf-z on 3/13/17.
// Update by tongda on 3/14/18.
//

#include "SM_Retention.hh"
#include "debug/EnergyMgmt.hh"
#include <fstream>

/******* SM_Retention *******/
SM_Retention::SM_Retention(const Params *p) : 
	BaseEnergySM(p), 
	state(SM_Retention::State::STATE_POWER_OFF),
	thres_ret_to_off(p->thres_ret_to_off),
	thres_1_to_ret(p->thres_1_to_ret),
	thres_ret_to_1(p->thres_ret_to_1)
{
	// when the system cannot consume energy
	energy_consume_lower_bound = thres_ret_to_off;
}

void 
SM_Retention::init()
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

void SM_Retention::update(double _energy)
{
	EnergyMsg msg;
	msg.val = 0;

	// power failure
	if ( (state == State::STATE_POWER_ON || state == State::STATE_POWER_RETENTION) 
		&& _energy <= thres_ret_to_off ) 
	{
		state = State::STATE_POWER_OFF;
		msg.type = MsgType::POWER_OFF;

		if (state == State::STATE_POWER_ON)
			DPRINTF(EnergyMgmt, "[SM_Retention] State change: POWER_ON->POWER_OFF, energy=%lf, thres=%lf.\n", _energy, thres_ret_to_off);
		else if (state == State::STATE_POWER_RETENTION)
			DPRINTF(EnergyMgmt, "[SM_Retention] State change: POWER_RET->POWER_OFF, energy=%lf, thres=%lf.\n", _energy, thres_ret_to_off);

		// Calculate Power failure times
		outage_times++;
		std::ofstream fout("m5out/power_failure");
		assert(fout);
		fout << outage_times << std::endl;
		fout.close();

		broadcastMsg(msg);
	} 

	// power retention
	else if (state == State::STATE_POWER_ON && _energy <= thres_1_to_ret)
	{
		state = State::STATE_POWER_RETENTION;
		msg.type = MsgType::POWER_RET;
		DPRINTF(EnergyMgmt, "[SM_Retention] State change: POWER_ON->POWER_RET, energy=%lf, thres=%lf.\n", _energy, thres_1_to_ret);
		broadcastMsg(msg);
	}

	// power recovery
	else if ( (state == State::STATE_POWER_OFF || state == State::STATE_POWER_RETENTION)
		&& _energy >= thres_ret_to_1) 
	{
		state = State::STATE_POWER_ON;
		msg.type = MsgType::POWER_ON;

		if (state == State::STATE_POWER_OFF)
			DPRINTF(EnergyMgmt, "[SM_Retention] State change: POWER_OFF->POWER_ON, energy=%lf, thres=%lf.\n", _energy, thres_ret_to_1);
		else if (state == State::STATE_POWER_RETENTION)
			DPRINTF(EnergyMgmt, "[SM_Retention] State change: POWER_RET->POWER_ON, energy=%lf, thres=%lf.\n", _energy, thres_ret_to_1);

		broadcastMsg(msg);
	}
}

SM_Retention *
SM_RetentionParams::create()
{
	return new SM_Retention(this);
}
