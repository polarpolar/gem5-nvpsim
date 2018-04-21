//
// Created by lf-z on 3/13/17.
//

#ifndef GEM5_SM_RETENTION_HH
#define GEM5_SM_RETENTION_HH

#include "sim/sim_object.hh"
#include "engy/energy_mgmt.hh"
#include "engy/state_machine.hh"
#include "params/BaseEnergySM.hh"
#include "params/SM_Retention.hh"

/********* Simple Energy SM ***********/
class SM_Retention : public BaseEnergySM
{
public:
	typedef SM_RetentionParams Params;
	const Params *params() const
	{
		return reinterpret_cast<const Params *>(_params);
	}
	SM_Retention(const Params *p);
	virtual ~SM_Retention() {}
	virtual void init();
	virtual void update(double _energy);

	// The states defined by energy managing algorithm
	enum State {
		STATE_POWER_OFF = 0,
		STATE_POWER_RETENTION = 1,
		STATE_POWER_ON = 2
	};

	// Messages
	enum MsgType {
		CONSUME_ENERGY = 0,
		POWER_OFF = 1,
		POWER_RETENTION = 2,
		POWER_ON = 2
	};


	int outage_times = 0;

protected:
	State state;
	double thres_ret_to_off;
	double thres_1_to_ret;
	double thres_off_to_1; 
};

#endif //GEM5_SM_RETENTION_HH