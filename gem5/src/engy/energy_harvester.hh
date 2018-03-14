//
// Created by lf-z on 3/26/17.
//

#ifndef GEM5_HARVEST_HH
#define GEM5_HARVEST_HH

#include "sim/sim_object.hh"
#include "params/BaseHarvester.hh"
#include "params/SimpleHarvester.hh"

class BaseHarvester : public SimObject
{
public:
    typedef BaseHarvesterParams Params;
    const Params *params() const
    {
        return reinterpret_cast<const Params *>(_params);
    }
    BaseHarvester(const Params *p) : SimObject(p) {}
    virtual ~BaseHarvester() {}
    virtual void init() {}
public:
    /* Return energy remained after harvesting energy */
    virtual double energy_harvest(double energy_harvested, double energy_remained)
    {
        return 0;
    }
};

class SimpleHarvester : public BaseHarvester
{
public:
    typedef SimpleHarvesterParams Params;
    const Params *params() const
    {
        return reinterpret_cast<const Params *>(_params);
    }
    SimpleHarvester(const Params *p) : BaseHarvester(p) {}
    virtual ~SimpleHarvester() {}
    virtual void init();
public:
    double energy_harvest(double energy_harvested, double energy_remained);
};

#endif //GEM5_HARVEST_HH
