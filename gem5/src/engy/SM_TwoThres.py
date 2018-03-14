from m5.SimObject import SimObject
from BaseEnergySM import BaseEnergySM
from m5.params import *
from m5.proxy import *

class SM_TwoThres(BaseEnergySM):
    type = 'SM_TwoThres'
    cxx_header = "engy/SM_TwoThres.hh"

    thres_1_to_off = Param.Float(Parent.thres_1_to_off ,"power off threshold")
    thres_off_to_1 = Param.Float(Parent.thres_off_to_1 ,"power on threshold")