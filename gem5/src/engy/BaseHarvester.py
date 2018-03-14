from m5.SimObject import SimObject
from m5.params import *
from m5.proxy import *

class BaseHarvester(SimObject):
    type = 'BaseHarvester'
    cxx_header = "engy/energy_harvester.hh"