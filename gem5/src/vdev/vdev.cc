//
// 	Source code file for Virtual Device
// 
//	Version 2.1.	12/3/2017, tongda
//		Add new states considering initializations
//		Add a retention -> power-off convertion in this part.
//	Perivious versions: ruoyang
//		Add access time and log need functions
//	Created by lf-z on 4/24/17
//

#include "vdev/vdev.hh"
#include "engy/state_machine.hh"
#include "debug/EnergyMgmt.hh"
#include "debug/VirtualDevice.hh"
#include "debug/MemoryAccess.hh"
#include "engy/DVFS.hh"
#include "engy/DFS_LRY.hh"

#include <fstream>


/** Definitions of TickEvent in Virtual Device **/
VirtualDevice::TickEvent::TickEvent(VirtualDevice *vdev_this)
    : Event(VirtualDevice_Tick_Pri), vdev(vdev_this)
{
}

void
VirtualDevice::TickEvent::process()
{
	assert(vdev);
	vdev->tick();
}

const char *
VirtualDevice::TickEvent::description() const
{
	assert(vdev);
	return "VirtualDevice tick";
}

/** Definitions of Device Port in Virtual Device **/
VirtualDevice::DevicePort::DevicePort(const std::string &_name, VirtualDevice *_vdev) : SlavePort(_name, _vdev), vdev(_vdev)
{

}

Tick
VirtualDevice::DevicePort::recvAtomic(PacketPtr pkt)
{
	assert(vdev);
	return vdev->recvAtomic(pkt);
}

void
VirtualDevice::DevicePort::recvFunctional(PacketPtr pkt)
{
	assert(vdev);
	vdev->recvFunctional(pkt);
}

bool
VirtualDevice::DevicePort::recvTimingReq(PacketPtr pkt)
{
	assert(vdev);
	return vdev->recvTimingReq(pkt);
}

void
VirtualDevice::DevicePort::recvRespRetry()
{
	assert(vdev);
	vdev->recvRespRetry();
}

AddrRangeList
VirtualDevice::DevicePort::getAddrRanges() const
{
	assert(vdev);
	AddrRangeList rangeList;
	rangeList.push_back(vdev->getAddrRange());
	return rangeList;
}

VirtualDevice::VirtualDevice(const Params *p) : 
	MemObject(p),
	id(0),
	tickEvent(this),
	port(name() + ".port", this),
	need_log(p->need_log),
	cpu(p->cpu),
	range(p->range),
	delay_set(p->delay_set),
	delay_self(p->delay_self),
	delay_recover(p->delay_recover),
	delay_cpu_interrupt(p->delay_cpu_interrupt),
	is_interruptable(p->is_interruptable),
	delay_remained(p->delay_remained + p->delay_recover),
	event_interrupt(this, false, Event::Virtual_Interrupt)
{
	trace.resize(0);
	pmem = (uint8_t*) malloc(range.size() * sizeof(uint8_t));
	memset(pmem, 0, range.size() * sizeof(uint8_t));
	
	// the energy consumption of each cycle defines the cost of three modes (OFF, SLEEP, ACTIVE)
	energy_consumed_per_cycle_vdev[0] = p->energy_consumed_per_cycle_vdev[0];
	energy_consumed_per_cycle_vdev[1] = p->energy_consumed_per_cycle_vdev[1];
	energy_consumed_per_cycle_vdev[2] = p->energy_consumed_per_cycle_vdev[2];
}

VirtualDevice::~VirtualDevice()
{
    if (tickEvent.scheduled()) {
        deschedule(tickEvent);
    }
}

void
VirtualDevice::init()
{
	MemObject::init();

	if (port.isConnected()) {
		port.sendRangeChange();
	}

	cpu->registerVDev(delay_recover, id);
	DPRINTF(VirtualDevice, "Virtual Device (id = %d) started with range: %#lx - %#lx\n", id, range.start(), range.end());

	vdev_energy_state = STATE_POWEROFF;
	if (!tickEvent.scheduled())
		schedule(tickEvent, clockEdge(Cycles(0)));
}

void
VirtualDevice::triggerInterrupt()
{
	/* interrupt return after initialization or activation */
	assert(*pmem & VDEV_BUSY);
	/* Change register byte. */
	*pmem |= VDEV_IDLE;
	*pmem &= ~VDEV_BUSY;
	vdev_energy_state = STATE_ACTIVE; // The virtual device enter/keep in the active status.
	DPRINTF(VirtualDevice, "Virtual device triggers an interrupt.\n");
	if (*pmem & VDEV_RAW) {
		DPRINTF(VirtualDevice, "Virtual device already initialized.\n");
		*pmem |= VDEV_READY;
		*pmem &= ~VDEV_RAW;
	} else {
		DPRINTF(VirtualDevice, "Virtual device task already completed.\n");	
	}
	finishSuccess();

	/* Tell cpu. */
	cpu->virtualDeviceInterrupt(delay_cpu_interrupt);
	cpu->virtualDeviceEnd(id);
}

Tick
VirtualDevice::access(PacketPtr pkt)
{
	/* Todo: what if the cpu ask to work on a task when the vdev is busy? **/
	DPRINTF(MemoryAccess, "Virtual Device accessed at %#lx.\n", pkt->getAddr());
	Addr offset = pkt->getAddr() - range.start();

	if (pkt->isRead()) {
		memcpy(pkt->getPtr<uint8_t>(), pmem+offset, pkt->getSize());
	} 
	else if (pkt->isWrite()) {
		/* the request will write the target address to 1 */
		const uint8_t* pkt_addr = pkt->getConstPtr<uint8_t>();
		if (offset == 0) {
			/* offset = 0, the address is in the cmd byte */
			/* Initialization */
			if (*pkt_addr & VDEV_INIT) {
				if (*pmem & VDEV_BUSY) {
					/* Request fails because the vdev is working. */
					DPRINTF(VirtualDevice, "State BUSY! Request discarded!\n");
				} else {
					vdev_energy_state = STATE_ACTIVE; // the virtual device enter/keep in the active status to complete the initialization
					DPRINTF(VirtualDevice, "Virtual Device starts initialization.\n");
					
					/* Set the virtual device to working mode */
					*pmem |= VDEV_BUSY;

					/* Schedule interrupt after init-delay. */
					schedule(event_interrupt, curTick() + delay_set);
					/* Energy consumption. */
					DPRINTF(VirtualDevice, "Need Ticks: %i, Cycles: %i, Energy: %lf .\n", 
						delay_set, 
						ticksToCycles(delay_set), 
						energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_set)
					);
					consumeEnergy(energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_set));
					cpu->virtualDeviceSet(delay_set);
					cpu->virtualDeviceStart(id);
				}
			}
			/* Activation. */
			else if (*pkt_addr & VDEV_ACTIVATE) {
				if (*pmem & VDEV_RAW) {
					/* virtual device not enabled before initialization */
					DPRINTF(VirtualDevice, "VirtualDevice not initialized! Request discarded!\n");
				}
				else if (*pmem & VDEV_BUSY) {
					/* Request fails because the vdev is working. */
					DPRINTF(VirtualDevice, "State BUSY! Request discarded!\n");
				} else {
					/* Request succeeds. */
					vdev_energy_state = STATE_ACTIVE; // The virtual device enter/keep in the active status.
					DPRINTF(VirtualDevice, "Virtual Device starts working.\n");
					
					/* 统计设备访问次数 */
					if (need_log)
					{
						access_time++;
						std::ofstream fout("m5out/devicedata");
						assert(fout);
						fout << access_time << std::endl;
						fout.close();
					}
					
					/* Set the virtual device to working mode */
					*pmem |= VDEV_BUSY;
					*pmem &= ~VDEV_IDLE;
					/* Schedule interrupt. */
					schedule(event_interrupt, curTick() + delay_self);
					/* Energy consumption. */
					DPRINTF(VirtualDevice, "Need Ticks:%i, Cycles:%i, Energy: %lf .\n", 
						delay_self, 
						ticksToCycles(delay_self), 
						energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_self)
					);
					consumeEnergy(energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_self));
					//cpu->virtualDeviceSet(delay_set);
					cpu->virtualDeviceStart(id);
				}
			} else {
				/* Not a request, but the first byte cannot be written. */
			}
		} else {
			/* Normal write. */
			memcpy(pmem+offset, pkt_addr, pkt->getSize());
		}
	}
	return 0;
}

void 
VirtualDevice::tick()
{
	// Task Latency
	Tick latency = clockPeriod();
	// Set the behavior in time tick grains
	if (vdev_energy_state != STATE_POWEROFF) {
		double EngyConsume = 0;
		// Energy Consumption
		EngyConsume = energy_consumed_per_cycle_vdev[vdev_energy_state] * ticksToCycles(latency);
		consumeEnergy(EngyConsume);
		// Info.
		switch(vdev_energy_state) {
			case STATE_POWEROFF:
				DPRINTF(VirtualDevice, "VirtualDevice id = %d, TickEvent@STATE_POWEROFF, latency = %d tick, Energy = %1f. \n", id, latency, EngyConsume);
			case STATE_SLEEP:
				DPRINTF(VirtualDevice, "VirtualDevice id = %d, TickEvent@STATE_SLEEP, latency = %d tick, Energy = %1f. \n", id, latency, EngyConsume);
			case STATE_ACTIVE:
				DPRINTF(VirtualDevice, "VirtualDevice id = %d, TickEvent@STATE_ACTIVE, latency = %d tick, Energy = %1f. \n", id, latency, EngyConsume);
			default:	
				DPRINTF(EnergyMgmt, "Unrecognized EngyMsg.\n");
				return;
		}
	}

	schedule(tickEvent, curTick() + latency);
}

int
VirtualDevice::handleMsg(const EnergyMsg &msg)
{
	DPRINTF(EnergyMgmt, "Device receives handleMsg at %lu, msg.type=%d\n", curTick(), msg.type);
	switch(msg.type) {
		case (int) DFS_LRY::MsgType::RETENTION_BEG:
			/** VDEV retention **
			  The virtual device energy mode turns to sleep (low cost), but all the uncompleted tasks are failed. Recover delay contains only 'delay_self' is it is un-interruptable.
			**/
			DPRINTF(EnergyMgmt, "The Msg is Enter-Retention.\n");
			vdev_energy_state = STATE_SLEEP;
			if (*pmem & VDEV_BUSY) {
				/* This should be handled if the device is on a task */
				assert(event_interrupt.scheduled());
				DPRINTF(VirtualDevice, "Device retention occurs in the middle of a task at %lu\n", curTick());
				DPRINTF(EnergyMgmt, "Device retention occurs in the middle of a task at %lu\n", curTick());

				/* Calculate the remaining delay*/
				if (!is_interruptable) {
					// the interrupted operation is initialization
					if (*pmem & VDEV_RAW)
						delay_remained = delay_set;
					// the interrupted operation is execution
					else
						delay_remained = delay_self;
				} else {
					delay_remained = event_interrupt.when() - curTick();
				}

				deschedule(event_interrupt);
			}
			break;
		case (int) DFS_LRY::MsgType::POWEROFF:
			/** VDEV shutdown **
			  The virtual device completed failed. It becomes un-initialized and needs re-initialization. If it is un-interruptable and fails during busy state, it also needs to restart and rerun the task with 'delay_self'. The device requires a recover time ('delay_recover') to support the reinitialization and restart procedure. 
			**/
			DPRINTF(EnergyMgmt, "The Msg is Enter-Power-Off.\n");
			/* Reset the states */
			*pmem |= VDEV_RAW;
			*pmem &= ~VDEV_READY;
			vdev_energy_state = STATE_POWEROFF;
			/* Todo: if entered from retention */
			if (vdev_energy_state == STATE_SLEEP) {
				assert(!event_interrupt.scheduled());
				DPRINTF(VirtualDevice, "device power off occurs during retention at %lu\n", curTick());
				DPRINTF(EnergyMgmt, "device power off occurs during retention at %lu\n", curTick());
				/* delay_remained updates with punishment */
				Tick delay_punish = 0;
				delay_remained += delay_recover + delay_set + delay_punish;
			} 
			/* if entered from active modes */
			else {
				delay_remained = delay_recover + delay_set;
				if (*pmem & VDEV_BUSY) {
					assert(event_interrupt.scheduled());
					DPRINTF(VirtualDevice, "device power off occurs in the middle of a task at %lu\n", curTick());
					if (!is_interruptable) {
						delay_remained += delay_self;
					} else {
						/*Todo: what if the peripheral is interruptable*/
						delay_remained += event_interrupt.when() - curTick();
					}
					deschedule(event_interrupt);
				}
			}	
			break;
		case (int) DFS_LRY::MsgType::RETENTION_END:
			/** VDEV recover from retention **/
			DPRINTF(EnergyMgmt, "The Msg is Retention-to-Active.\n");
			vdev_energy_state = STATE_ACTIVE;
			if (*pmem & VDEV_BUSY) 
			{
				assert(!event_interrupt.scheduled());
				DPRINTF(VirtualDevice, "device recover from retention to finish a task at %lu\n", curTick());
				schedule(event_interrupt, curTick() + delay_remained);
				/* Energy consumption. */
				DPRINTF(VirtualDevice, "Need Ticks: %i, Cycles: %i, Energy: %lf .\n", 
					delay_remained, 
					ticksToCycles(delay_remained), 
					energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained)
				);
				consumeEnergy(energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained));
				cpu->virtualDeviceStart(id);
			}
			break;
		case (int) DFS_LRY::MsgType::POWERON:
			/** VDEV power recover **/
			DPRINTF(EnergyMgmt, "The Msg is Poweroff-to-Active.\n");
			vdev_energy_state = STATE_ACTIVE;
			if (*pmem & VDEV_BUSY) {
				assert(!event_interrupt.scheduled());
				DPRINTF(VirtualDevice, "device power on to finish a task at %lu\n", curTick());
				schedule(event_interrupt, curTick() + delay_remained);
				/** Energy consumption **/
				DPRINTF(VirtualDevice, "Need Ticks: %i, Cycles: %i, Energy: %lf .\n", 
					delay_remained, 
					ticksToCycles(delay_remained), 
					energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained)
				);
				consumeEnergy(energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained));
				cpu->virtualDeviceStart(id);
			}
			break;
		default:
			DPRINTF(EnergyMgmt, "Unrecognized EngyMsg.\n");
			return 0;
	}
	return 1;
}

BaseSlavePort&
VirtualDevice::getSlavePort(const std::string &if_name, PortID idx)
{
	if (if_name == "port") {
		return port;
	} else {
		return MemObject::getSlavePort(if_name, idx);
	}
}

AddrRange
VirtualDevice::getAddrRange() const
{
	return range;
}

Tick
VirtualDevice::recvAtomic(PacketPtr pkt)
{
	return access(pkt);
}

void
VirtualDevice::recvFunctional(PacketPtr pkt)
{
	fatal("Functional access is not supported now.");
}

bool
VirtualDevice::recvTimingReq(PacketPtr pkt)
{
	fatal("Timing access is not supported now.");
}

void
VirtualDevice::recvRespRetry()
{
	fatal("Timing access is not supported now.");
}

bool
VirtualDevice::finishSuccess()
{
	/* Todo: Need further implementation. */
	return 1;
}

VirtualDevice *
VirtualDeviceParams::create()
{
	return new VirtualDevice(this);
}
