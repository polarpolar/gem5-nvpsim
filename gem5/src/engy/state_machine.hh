//
// Created by lf-z on 3/13/17.
//

#ifndef GEM5_STATE_MACHINE_HH
#define GEM5_STATE_MACHINE_HH

#include "sim/sim_object.hh"
#include "engy/energy_mgmt.hh"
#include "params/BaseEnergySM.hh"
#include "params/SimpleEnergySM.hh"

class EnergyMgmt;

/********* Basic Energy SM ***********/
class BaseEnergySM : public SimObject
{
	/* Set EnergyMgmt as friend class so that this->mgmt can be initialized. */
	friend class EnergyMgmt;
public:
	typedef	 BaseEnergySMParams Params;
	const Params *params() const
	{
		return reinterpret_cast<const Params *>(_params);
	}
	BaseEnergySM(const Params *p);
	virtual ~BaseEnergySM() {}
	virtual void init() {}
	virtual void update(double _energy) {}

protected:
	EnergyMgmt *mgmt;
	void broadcastMsg(const EnergyMsg &msg);
	double energy_consume_lower_bound;
};

/********* Simple Energy SM ***********/
class SimpleEnergySM : public BaseEnergySM
{
public:
	typedef SimpleEnergySMParams Params;
	const Params *params() const
	{
		return reinterpret_cast<const Params *>(_params);
	}
	SimpleEnergySM(const Params *p);
	virtual ~SimpleEnergySM() {}
	virtual void init();
	virtual void update(double _energy);

	// The states defined by energy managing algorithm
	enum State {
		STATE_POWER_OFF = 0,
		STATE_POWER_ON = 1
	};

	// Messages
	enum MsgType {
		CONSUME_ENERGY = 0,
		POWER_OFF = 1,
		POWER_ON = 2
	};


	int outage_times = 0;

protected:
	State state;
	double thres_1_to_off;
	double thres_off_to_1; 
};

#endif //GEM5_STATE_MACHINE_HH