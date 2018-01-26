from m5.params import *
from MemObject import MemObject
from AtomicSimpleCPU import AtomicSimpleCPU

class VirtualDevice(MemObject):
    type = 'VirtualDevice'
    cxx_header = 'vdev/vdev.hh'
    port = SlavePort("Slave port of virtual device.")
    id = Param.Int('0', "Id of the virtual device.")
    delay_self = Param.Clock('1ms', "delay of device to finish its mission")
    delay_set = Param.Clock('0t', "delay to set up the device")
    delay_recover = Param.Clock('0t', "time that the cpu spend to recover the device after a power failure")
    delay_remained = Param.Clock('1ms', "time that the device spend after power failure to finish the task")
    delay_cpu_interrupt = Param.Clock('1ms', "time that the cpu need to take when the device finishes its work")
    is_interruptable = Param.Bool(0, "whether the device is interruptable")
    cpu = Param.BaseCPU(NULL, "The cpu of the system")
    range = Param.AddrRange('1MB', "Address range")
    energy_consumed_per_cycle_vdev = VectorParam.Float([], "The power-off, idle and active energy consumption, where the first one (power-off consumption) must be zero.")
    need_log = Param.Bool(0, "whether this device needs log how many times it is triggered to \"m5out/devicedata\"")
