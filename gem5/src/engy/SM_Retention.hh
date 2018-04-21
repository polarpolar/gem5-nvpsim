//
// Created by lf-z on 3/13/17.
//

#ifndef GEM5_SM_RETENTION_HH
#define GEM5_SM_RETENTION_HH

#include "engy/state_machine.hh"
#include "params/SM_Retention.hh"

/********* State-Retention SM ***********/
class SM_Retention : public BaseEnergySM
{
public:
	typedef SM_RetentionParams Params;
	const Params *params() const
	{
		return reinterpret_cast<const Params *>(_params);
	}
	SM_Retention(const Params *p);
	~SM_Retention() {}
	virtual void init();
	virtual void update(double _energy);

	enum State {
		STATE_POWER_OFF = 0,
		STATE_POWER_RETENTION = 1,
		STATE_POWER_ON = 2
	};

	enum MsgType {
		CONSUME_ENERGY = 0,
		POWER_OFF = 1,
		POWER_RET = 2,
		POWER_ON = 3
	};


	int outage_times = 0;

protected:
	State state;
	double thres_ret_to_off;
	double thres_1_to_ret;
	double thres_ret_to_1; 
};

#endif //GEM5_SM_RETENTION_HH