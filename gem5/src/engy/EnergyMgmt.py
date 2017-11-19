from m5.SimObject import SimObject
from m5.params import *
from m5.proxy import *
from SimpleEnergySM import SimpleEnergySM
from SimpleHarvest import SimpleHarvest

class EnergyMgmt(SimObject):
    type = 'EnergyMgmt'
    cxx_header = "engy/energy_mgmt.hh"
    path_energy_profile = Param.String(Parent.path_energy_profile, "path to energy profile")
    energy_time_unit = Param.Clock(Parent.energy_time_unit, "time unit for energy profile")
    state_machine = Param.BaseEnergySM(SimpleEnergySM(), "energy state machine")
    harvest_module = Param.BaseHarvest(SimpleHarvest(), "energy harvesting module")
    capacity = Param.Float(1.0, "capacitor (1uF for default)")
    energy_consumed_per_harvest = Param.Float(0.01, "energy consumed per harvest (because of leakage)")
    energy_profile_mult = Param.Float(1, "energy harvest multiplier")