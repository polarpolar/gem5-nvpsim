import m5
from m5.objects import *

import os
if os.path.exists("m5out/devicedata"):
	os.remove("m5out/devicedata")

import sys  
cap = float(sys.argv[1])*30
profilemult = float(sys.argv[2])/10

#ticks_per_sec = 1000000000000
#max_sec = 100

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1MHz'
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'atomic'
system.mem_ranges = [AddrRange('512MB')]

#vdev
system.has_vdev = 1
system.vdev_ranges = [AddrRange('512MB', '512MB'), AddrRange('513MB', '513MB'), AddrRange('514MB', '514MB')]
system.vaddr_vdev_ranges = [AddrRange('1000MB', '1000MB'), AddrRange('1001MB', '1001MB'), AddrRange('1002MB', '1002MB')]
###

#energy mgmt
system.energy_mgmt = EnergyMgmt(path_energy_profile = 'profile/powerofficeuw1e4.txt', energy_time_unit = '10us')
system.energy_mgmt.state_machine = DFS_LRY()
system.energy_mgmt.state_machine.thres_5_to_4 = 1000000000000
system.energy_mgmt.state_machine.thres_4_to_3 = 1000000000000
system.energy_mgmt.state_machine.thres_3_to_2 = 1000000000000
system.energy_mgmt.state_machine.thres_2_to_1 = 1000000000000
system.energy_mgmt.state_machine.thres_1_to_retention = 0.5 * cap * 1000
system.energy_mgmt.state_machine.thres_retention_to_off = 0.5 * cap * 1000 * 2

system.energy_mgmt.state_machine.thres_off_to_1 = 0.5 * cap * 1000 * 4.5 *4.5
system.energy_mgmt.state_machine.thres_retention_to_1 = 0.5 * cap * 1000 * 4.5 *4.5
system.energy_mgmt.state_machine.thres_1_to_2 = 1000000000000
system.energy_mgmt.state_machine.thres_2_to_3 = 1000000000000
system.energy_mgmt.state_machine.thres_3_to_4 = 1000000000000
system.energy_mgmt.state_machine.thres_4_to_5 = 1000000000000

system.energy_mgmt.capacity = cap;	#uF
system.energy_mgmt.energy_consumed_per_harvest = 0.02; 
system.energy_mgmt.energy_profile_mult = profilemult; 
###

#set some parameters for the CPU
system.cpu = AtomicSimpleCPU(energy_consumed_per_cycle_5 = 1000000000000,
                             energy_consumed_per_cycle_4 = 1000000000000,
                             energy_consumed_per_cycle_3 = 1000000000000,
                             energy_consumed_per_cycle_2 = 2,
                             energy_consumed_per_cycle_1 = 1,
	  
                             energy_consumed_poweron = 50,  
                             clockPeriod_to_poweron = 10,
	  
                             clock_mult_5 = 1/1000000000000,
                             clock_mult_4 = 1/1000000000000,
                             clock_mult_3 = 1/1000000000000,
                             clock_mult_2 = 1/1.5,
                             clock_mult_1 = 1/0.75)

system.cpu.s_energy_port = system.energy_mgmt.m_energy_port

system.membus = SystemXBar()

system.cpu.icache_port = system.membus.slave
system.cpu.dcache_port = system.membus.slave

system.cpu.createInterruptController()

system.mem_ctrl = DDR3_1600_x64()
system.mem_ctrl.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.master

system.system_port = system.membus.slave

system.vdev1 = VirtualDevice()
system.vdev1.cpu = system.cpu
system.vdev1.range = system.vdev_ranges[0]
system.vdev1.energy_consumed_per_cycle_vdev = [Float(0), Float(0.003), Float(1.35)] # The energy consumption of each cycle at power-off, idle and active mode.
system.vdev1.delay_self = '10ms'
system.vdev1.delay_cpu_interrupt = '100us'
system.vdev1.delay_set = '2200us'
system.vdev1.delay_recover = '920us'
system.vdev1.is_interruptable = 0
system.vdev1.port = system.membus.master
system.vdev1.s_energy_port = system.energy_mgmt.m_energy_port

system.vdev2 = VirtualDevice()
system.vdev2.cpu = system.cpu
system.vdev2.range = system.vdev_ranges[1]
system.vdev2.energy_consumed_per_cycle_vdev = [Float(0.0234), Float(0.26), Float(0.416)] # The energy consumption of each cycle at power-off, idle and active mode.
system.vdev2.delay_self = '10ms'
system.vdev2.delay_cpu_interrupt = '100us'
system.vdev2.delay_set = '1281us'
system.vdev2.delay_recover = '492us'
system.vdev2.is_interruptable = 0
system.vdev2.port = system.membus.master
system.vdev2.s_energy_port = system.energy_mgmt.m_energy_port

system.vdev3 = VirtualDevice()
system.vdev3.cpu = system.cpu
system.vdev3.range = system.vdev_ranges[2]
system.vdev3.energy_consumed_per_cycle_vdev = [Float(0), Float(11.9), Float(11.9)] # The energy consumption of each cycle at power-off, idle and active mode.
system.vdev3.delay_self = '100us'
system.vdev3.delay_cpu_interrupt = '100us'
system.vdev3.delay_set = '660us'
system.vdev3.delay_recover = '145us'
system.vdev3.is_interruptable = 0
system.vdev3.port = system.membus.master
system.vdev3.s_energy_port = system.energy_mgmt.m_energy_port

system.vdev3.need_log = 1

process = LiveProcess()
process.cmd = ['tests/test-progs/brgMonitor/main_trans']
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print "Beginning simulation! cap: %f" % cap
#exit_event = m5.simulate(int(max_sec * ticks_per_sec))
exit_event = m5.simulate()
print 'Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause())

if os.path.exists("m5out/devicedata"):
	fi = open("m5out/devicedata","r")
	line = fi.readline()
	vdev_access = int(line)
	fi.close()
else:
	vdev_access = 0

fo = open("m5out/batch_res.csv","a")
fo.write("%f,%f,%i,%i,%s\n" % (cap, profilemult, vdev_access, m5.curTick(), exit_event.getCause()))
fo.close()

print "%f,%f,%i,%i" % (cap, profilemult, vdev_access, m5.curTick())