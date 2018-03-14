//
// Created by lf-z on 3/13/17.
// Update by tongda on 3/14/18.
//

#include "engy/state_machine.hh"
#include "debug/EnergyMgmt.hh"


int
AtomicSimpleCPU::handleMsg(const EnergyMsg &msg) 
{
	int rlt = 1;
	Tick lat = 0;
	DPRINTF(EnergyMgmt, "[AtomicSimpleNVP] handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);

	switch(msg.type)
	{
		case (int) MsgType::POWER_OFF:
			// Todo: Add backup function here
			lat = tickEvent.when() - curTick();
			if (in_interrupt)
				lat_poweron = lat + clockPeriod() - lat % clockPeriod();
			else
				lat_poweron = 0;
			deschedule(tickEvent);
			break;

		case (int) MsgType::POWER_ON:
			consumeEnergy(energy_consumed_per_cycle * ticksToCycles(lat_poweron + BaseCPU::getTotalLat()));
			schedule(tickEvent, curTick() + lat_poweron + BaseCPU::getTotalLat());
			break;

		default: 
			return 0;
	}

	return 1;
}