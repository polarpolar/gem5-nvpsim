//
// Created by lf-z on 3/26/17.
//
#include "engy/energy_harvester.hh"
#include "debug/EnergyMgmt.hh"

void 
SimpleHarvester::init()
{
    DPRINTF(EnergyMgmt, "[EngyHarvest] SimpleHarvesterer is used in this system.\n");
}

double 
SimpleHarvester::energy_harvest(double energy_harvested, double energy_remained)
{
    return energy_remained + energy_harvested;
}

/******* Create python object *********/
BaseHarvester *
BaseHarvesterParams::create()
{
    return new BaseHarvester(this);
}

SimpleHarvester *
SimpleHarvesterParams::create()
{
    return new SimpleHarvester(this);
}
