from m5.SimObject import SimObject
from BaseHarvester import BaseHarvester
from m5.params import *
from m5.proxy import *

class SimpleHarvester(BaseHarvester):
    type = 'SimpleHarvester'
    cxx_header = "engy/energy_harvester.hh"