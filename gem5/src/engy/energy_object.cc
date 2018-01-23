//
// Created by lf-z on 1/9/17.
//

#include <string.h>
#include "engy/energy_object.hh"
#include "engy/energy_port.hh"

void EnergyObject::setSimObject(SimObject *_sim)
{
    _seport.setOwner(_sim);
    _meport.setOwner(_sim);
}
int EnergyObject::consumeEnergy(char *sender, double _energy)
{
    EnergyMsg msg;
    /* Zero is the default value of msg.type, indicating energy consuming*/
    msg.type = 0;
    msg.sender = (char*)malloc(sizeof(char)*100);
    strcpy(msg.sender, sender);
    msg.val = _energy;
    _seport.signalMsg(msg);
    return 1;
}

MasterEnergyPort& EnergyObject::getMasterEnergyPort()
{
    return _meport;
}

SlaveEnergyPort& EnergyObject::getSlaveEnergyPort()
{
    return _seport;
}

int EnergyObject::handleMsg(const EnergyMsg &msg)
{
    return 1;
}