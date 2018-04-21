//
// Created by lf-z on 3/13/17.
//

#ifndef SM_TWOTHRES_HH
#define SM_TWOTHRES_HH

#include "state_machine.hh"
#include "params/SM_TwoThres.hh"

/********* Two Thresholds SM ***********/
class SM_TwoThres : public BaseEnergySM
{
public:
	typedef SM_TwoThresParams Params;
	const Params *params() const
	{
		return reinterpret_cast<const Params *>(_params);
	}
	SM_TwoThres(const Params *p);
	~SM_TwoThres() {}
	virtual void init();
	virtual void update(double _energy);

	enum State
	{
		STATE_POWEROFF = 0,
		STATE_POWERON = 1
	};

	enum MsgType {
		MSG_CONSUMEENERGY = 0,
		MSG_POWEROFF = 1,
		MSG_POWERON = 2
	};

protected:
	State state;
	double thres_1_to_off;
	double thres_off_to_1; 
};

#endif	// SM_TWOTHRES_HH