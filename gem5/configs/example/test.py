import m5
from m5.objects import *

import sys 

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1MHz'
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'atomic'
system.mem_ranges = [AddrRange('512MB')]

# CPU: basic params
system.cpu = AtomicSimpleCPU()

# CPU: memory
system.membus = SystemXBar()
system.cpu.icache_port = system.membus.slave
system.cpu.dcache_port = system.membus.slave
system.cpu.createInterruptController()
system.mem_ctrl = DDR3_1600_x64()
system.mem_ctrl.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.master
system.system_port = system.membus.slave

process = LiveProcess()
process.cmd = ['tests/test-progs/brgMonitor/main_trans']
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print "Beginning simulation!"
exit_event = m5.simulate(int(0.2 * 1000000000000))
print 'Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause())