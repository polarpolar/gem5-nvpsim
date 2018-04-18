import m5
from m5.objects import *

import sys  
cap = float(sys.argv[1])
profilemult = float(sys.argv[2])
print "cap: %f; energy: %f.\n" %(cap, profilemult)
#cap = 10 		# uF
#profilemult = 10	# times

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1MHz'
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'atomic'
system.mem_ranges = [AddrRange('512MB')]

#vdev
system.has_vdev = 1
system.vdev_ranges = [AddrRange('512MB', '512MB'), 
			AddrRange('513MB', '513MB'), 
			AddrRange('514MB', '514MB'), 
			AddrRange('515MB', '515MB'), 
			AddrRange('516MB', '516MB')]
system.vaddr_vdev_ranges = [AddrRange('1000MB', '1000MB'), 
			AddrRange('1001MB', '1001MB'), 
			AddrRange('1002MB', '1002MB'), 
			AddrRange('1003MB', '1003MB'), 
			AddrRange('1004MB', '1004MB')]

###################################
#####	Energy Management Profiles #####
###################################

# Power Supply (file path and sample period)
system.energy_mgmt = EnergyMgmt(path_energy_profile = 'profile/solar_new2.txt', energy_time_unit = '10us')
# Energy Management Strategy: State Machine
system.energy_mgmt.state_machine = SimpleEnergySM()
# Threshold Design for the state machine
system.energy_mgmt.state_machine.thres_1_to_off = 0.5 * cap * 1000 * 1.1 * 1.1
system.energy_mgmt.state_machine.thres_off_to_1 = 0.5 * cap * 1000 * 4.5 *4.5
# Energy Storage and leakage design of the capacitor
system.energy_mgmt.capacity = cap;				# uF
system.energy_mgmt.system_leakage = 0.2;			# leakage
system.energy_mgmt.energy_profile_mult = profilemult; 	# adjust the energy

print "---- Full cap: %f." %(0.5 * cap * 1000 * 5 * 5)
print "---- thres_1_to_off: %f." %(system.energy_mgmt.state_machine.thres_1_to_off)
print "---- thres_off_to_1: %f." %(system.energy_mgmt.state_machine.thres_off_to_1)
print "---- deltaE = %f.\n" %(0.5 * cap * 1000 * ( (system.energy_mgmt.state_machine.thres_1_to_off*system.energy_mgmt.state_machine.thres_1_to_off) - (system.energy_mgmt.state_machine.thres_off_to_1*system.energy_mgmt.state_machine.thres_off_to_1) ) )

###################################
##########	CPU 	###############
###################################

# CPU: basic params
system.cpu = AtomicSimpleCPU(
			power_cpu = 1.3, 	# nJ/cycle
			cycle_backup = 5, 	# nJ/cycle
			cycle_restore = 3 	# nJ/cycle
		)
# CPU: slave port
system.cpu.s_energy_port = system.energy_mgmt.m_energy_port
# CPU: memory
system.membus = SystemXBar()
system.cpu.icache_port = system.membus.slave
system.cpu.dcache_port = system.membus.slave
system.cpu.createInterruptController()
system.mem_ctrl = DDR3_1600_x64()
system.mem_ctrl.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.master
system.system_port = system.membus.slave

###################################
##########  Virtual Device  ##########
###################################

# Virtual device #0
system.vdev0 = VirtualDevice()
system.vdev0.id = 0;
system.vdev0.cpu = system.cpu
# Access address range for the device
system.vdev0.range = system.vdev_ranges[0]
# The energy consumption of each cycle at power-off, idle and active mode.
system.vdev0.energy_consumed_per_cycle_vdev = [Float(0), Float(0.03), Float(0.6), Float(1.35)] 
# Delay of an active task
system.vdev0.delay_self = '1ms'
# Delay of the task returning interrupt
system.vdev0.delay_cpu_interrupt = '100us'
# Initialization delay
system.vdev0.delay_set = '2200us'
# Recovering delay :: ToRemove
system.vdev0.delay_recover = '920us'
# The device is volatile (is_interruptable = 0)
system.vdev0.is_interruptable = 0
# Function and energy interface to connect to the system bus
system.vdev0.port = system.membus.master
system.vdev0.s_energy_port = system.energy_mgmt.m_energy_port
# Generate log file of this device
system.vdev0.need_log = 1

system.vdev1 = VirtualDevice()
system.vdev1.id = 1;
system.vdev1.cpu = system.cpu
system.vdev1.range = system.vdev_ranges[1]
system.vdev1.energy_consumed_per_cycle_vdev = [Float(0), Float(0.26), Float(0.416), Float(0.416)]
system.vdev1.delay_self = '1ms'
system.vdev1.delay_cpu_interrupt = '100us'
system.vdev1.delay_set = '1281us'
system.vdev1.delay_recover = '492us'
system.vdev1.is_interruptable = 0
system.vdev1.port = system.membus.master
system.vdev1.s_energy_port = system.energy_mgmt.m_energy_port

system.vdev2 = VirtualDevice()
system.vdev2.id = 2;
system.vdev2.cpu = system.cpu
system.vdev2.range = system.vdev_ranges[2]
system.vdev2.energy_consumed_per_cycle_vdev = [Float(0), Float(0.12), Float(2.4), Float(11.9)]
system.vdev2.delay_self = '10us'
system.vdev2.delay_cpu_interrupt = '10us'
system.vdev2.delay_set = '66us'
system.vdev2.delay_recover = '145us'
system.vdev2.is_interruptable = 0
system.vdev2.port = system.membus.master
system.vdev2.s_energy_port = system.energy_mgmt.m_energy_port

system.vdev3 = VirtualDevice()
system.vdev3.id = 3;
system.vdev3.cpu = system.cpu
system.vdev3.range = system.vdev_ranges[3]
system.vdev3.energy_consumed_per_cycle_vdev = [Float(0), Float(0.12), Float(2.4), Float(11.9)]
system.vdev3.delay_self = '10us'
system.vdev3.delay_cpu_interrupt = '10us'
system.vdev3.delay_set = '66us'
system.vdev3.delay_recover = '145us'
system.vdev3.is_interruptable = 0
system.vdev3.port = system.membus.master
system.vdev3.s_energy_port = system.energy_mgmt.m_energy_port

system.vdev4 = VirtualDevice()
system.vdev4.id = 4;
system.vdev4.cpu = system.cpu
system.vdev4.range = system.vdev_ranges[4]
system.vdev4.energy_consumed_per_cycle_vdev = [Float(0), Float(0.26), Float(2.4), Float(0.416)]
system.vdev4.delay_self = '1ms'
system.vdev4.delay_cpu_interrupt = '100us'
system.vdev4.delay_set = '1281us'
system.vdev4.delay_recover = '492us'
system.vdev4.is_interruptable = 0
system.vdev4.port = system.membus.master
system.vdev4.s_energy_port = system.energy_mgmt.m_energy_port
system.vdev4.need_log = 1



###################################
###########  Benchmark  ############
###################################

process = LiveProcess()
# Benchmark path
process.cmd = ['tests/test-progs/brgMonitor/main_trans']
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print "Beginning simulation!"
exit_event = m5.simulate(int(10 * 1000000000000))
print 'Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause())


#fi = open("m5out/devicedata","r")
#line = fi.readline()
#vdev_access = int(line)
#print "vdev3 access: %i" % vdev_access
#fi.close()