//
// 	Source code file for Virtual Device
// 
//	Version NVRF	20/4/2018, tongda
// 		Add
//	Version 2.1.	12/3/2017, tongda
//		Add new states considering initializations
//		Add a retention -> power-off convertion in this part.
//	Perivious versions: ruoyang
//		Add access time and log need functions
//	Created by lf-z on 4/24/17
//

#include <string.h>
#include "vdev/vdev.hh"
#include "engy/state_machine.hh"
#include "engy/SM_Retention.hh"
#include "debug/EnergyMgmt.hh"
#include "debug/VirtualDevice.hh"
#include "debug/MemoryAccess.hh"

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
	id(p->id),
	need_recover(false),
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
	delay_remained(p->delay_remained),
	vdev_energy_state(VdevEngyState::STATE_POWER_OFF),
	event_interrupt(this, false, Event::Virtual_Interrupt),
	event_init(this, false, Event::Virtual_Interrupt)
{
	sprintf(dev_name, "VDEV-%d", id);
	trace.resize(0);
	pmem = (uint8_t*) malloc(range.size() * sizeof(uint8_t));
	memset(pmem, 0, range.size() * sizeof(uint8_t));

	// delay_remained is zeros at the beginning
	delay_remained = 0;
	
	// the energy consumption of each cycle defines the cost of three modes (OFF, SLEEP, NORMAL, ACTIVE)
	energy_consumed_per_cycle_vdev[0] = p->energy_consumed_per_cycle_vdev[0];
	energy_consumed_per_cycle_vdev[1] = p->energy_consumed_per_cycle_vdev[1];
	energy_consumed_per_cycle_vdev[2] = p->energy_consumed_per_cycle_vdev[2];
	energy_consumed_per_cycle_vdev[3] = p->energy_consumed_per_cycle_vdev[3];
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
	// Memory port
	if (port.isConnected()) {
		port.sendRangeChange();
	}

	cpu->registerVDev(delay_recover, id);
	DPRINTF(VirtualDevice, "VDEV(%d) name: %s, range: %#lx - %#lx.\n", id, dev_name, range.start(), range.end());

	vdev_energy_state = VdevEngyState::STATE_POWER_OFF;
	if (!tickEvent.scheduled())
		schedule(tickEvent, clockEdge(Cycles(0)));
}

// Called when the vdev complete an interrupt.
void
VirtualDevice::triggerInterrupt()
{
	/* interrupt return after initialization or activation */
	DPRINTF(VirtualDevice, "%s: Triggers an interrupt.\n", dev_name);
	assert(*pmem & VDEV_BUSY);
	/* Change register byte. */
	*pmem |= VDEV_IDLE;
	*pmem &= ~VDEV_BUSY;
	vdev_energy_state = VdevEngyState::STATE_NORMAL;

	// Initialization operation return
	if (*pmem & VDEV_CHAOS) {
		DPRINTF(VirtualDevice, "%s: Already initialized.\n", dev_name);
		*pmem |= VDEV_READY;
		*pmem &= ~VDEV_CHAOS;
		cpu->virtualDeviceInterrupt(dev_name, 0);
		cpu->virtualDeviceEnd(id);

		// Todo: these are external added codes, which should be added in extension parts.
		// if delay_remained > 0, means that a restart task is remaining
		if (delay_remained > 0) {
			DPRINTF(VirtualDevice, "%s: Restart-Task, Need Ticks: %i, Energy: %lf.\n", dev_name, delay_remained, 					energy_consumed_per_cycle_vdev[VdevEngyState::STATE_ACTIVE] * ticksToCycles(delay_remained)
				);
			// Schedule the new tasks
			schedule(event_interrupt, curTick() + delay_remained);
			// Set vdev state: Busy again
			*pmem |= VDEV_BUSY;
			// Set engy state: Active
			vdev_energy_state = VdevEngyState::STATE_ACTIVE;
			// Alarm the cpu to wait
			cpu->virtualDeviceRecover(dev_name, delay_remained);
			cpu->virtualDeviceStart(id);
			// reset delay_remained
			delay_remained = 0;
		}

	// Actuation operation return
	} else {
		DPRINTF(VirtualDevice, "%s: Completed the task.\n", dev_name);	

		// if delay_remained > 0, means that, the completed task is a recovered actuation.
		delay_remained = 0;

		cpu->virtualDeviceInterrupt(dev_name, delay_cpu_interrupt);
		cpu->virtualDeviceEnd(id);
	}
	// finishSuccess();
}

// Todo: What is the correct peripheral model with four voltage steps?
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
					// Vdev enters/keeps ACTIVE to complete the initialization
					vdev_energy_state = VdevEngyState::STATE_NORMAL; 
					
					/* Set the virtual device to working mode */
					*pmem |= VDEV_BUSY;
					*pmem &= ~VDEV_IDLE;

					/* Schedule interrupt after init-delay. */
					schedule(event_init, curTick() + delay_set);
					/* Initialization Information. */
					DPRINTF(VirtualDevice, "%s: Initialization. Need Ticks: %i, Energy: %lf.\n", 
						dev_name, delay_set, 
						energy_consumed_per_cycle_vdev[VdevEngyState::STATE_NORMAL] * ticksToCycles(delay_set)
					);
				}
			}

			/* Activation. */
			else if (*pkt_addr & VDEV_ACTIVATE) {
				if (*pmem & VDEV_CHAOS) {
					/* virtual device not enabled before initialization */
					DPRINTF(VirtualDevice, "VirtualDevice not initialized! Request discarded!\n");
				}
				else if (*pmem & VDEV_BUSY) {
					/* Request fails because the vdev is working. */
					DPRINTF(VirtualDevice, "State BUSY! Request discarded!\n");
				} else {
					/* Request succeeds. */
					vdev_energy_state = VdevEngyState::STATE_ACTIVE; // The virtual device enter/keep in the active status.
					
					/* 统计设备访问次数 */
					if (need_log) {
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
					DPRINTF(VirtualDevice, "%s: Start working. Need Ticks:%i, Energy: %lf.\n", 
						dev_name, delay_self, 
						energy_consumed_per_cycle_vdev[VdevEngyState::STATE_ACTIVE] * ticksToCycles(delay_self)
					);
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
	double EngyConsume = 0;
	EngyConsume = energy_consumed_per_cycle_vdev[vdev_energy_state] * ticksToCycles(latency);
	// Todo: where did we declare the EnergyMgmt Obj.?
	// EnergyMgmt::consumeEnergy() to realize energy consumption.
	EnergyObject::consumeEnergy(dev_name, EngyConsume);
	// scheduler the next vdev::tickEvent to EventQueue
	schedule(tickEvent, curTick() + latency);
}

// Todo: the handler is also related to peri. model.
int
VirtualDevice::handleMsg(const EnergyMsg &msg)
{
	/* Note: Vdev device is now a passive device which is 	*/
	/* not defined by different state machines. Now this 		*/
	/* device only reacts for two energy messages, that is:	*/
	/* 	power off: directly fail.				*/
	/* 	power on: energy state recovery.			*/

	// Power-off: the device fails
	if (msg.type == SM_Retention::MsgType::POWER_OFF) 
	{	
		// In state-retention strategy, Power failure means restart from the very beginning
		// Set energy state
		vdev_energy_state = VdevEngyState::STATE_POWER_OFF;
		// Reset vdev to be uninitialized.
		*pmem |= VDEV_CHAOS;
		*pmem &= ~VDEV_READY;
		*pmem &= ~VDEV_BUSY;
		*pmem &= ~VDEV_IDLE;

		// EXTENTION: User defined behaviors
	}

	// User defined Msgs: Retention
	else if (msg.type == SM_Retention::MsgType::POWER_RET) {

		// Ready but not busy: no task on working --> no behavior
		if ( (*pmem & VDEV_READY) && !(*pmem & VDEV_BUSY) ) {
			DPRINTF(VirtualDevice, "%s: Power-retention, ready but not busy (idle).\n", dev_name);
			// No recovery
			need_recover = false;
			// Not task needs to be restart
			delay_remained = 0;
		} 

		// Not ready but busy: init task on working --> restart initialization
		else if	( !(*pmem & VDEV_READY) && (*pmem & VDEV_BUSY) ) {
			DPRINTF(VirtualDevice, "%s: Power-retention, not ready but busy (init-ing).\n", dev_name);
			// remove the incomplete initialization task
			assert(event_init.scheduled());
			deschedule(event_init);
			// Need to recover the initialization
			need_recover = true;
			// Not peri. task needs to be restart
			delay_remained = 0;
		}

		// Ready and busy: peri. task on working --> restart task without reinitialization
		else if ( (*pmem & VDEV_READY) && (*pmem & VDEV_BUSY) ) {	
			DPRINTF(VirtualDevice, "%s: Power-retention, ready and busy (working).\n", dev_name);
			// remove task
			assert(event_interrupt.scheduled());
			deschedule(event_interrupt);
			// Need to recover only the peri. task
			need_recover = true;
			/* Calculate the remaining delay*/
			if (!is_interruptable) {
				// the disrupted task need to be recovered completed
				delay_remained = delay_self;
			} else {
				// the disrupted task can continue after that
				delay_remained = event_interrupt.when() - curTick();
			}
		}

		// After retention: sleep mode, not busy
		// Set energy state
		vdev_energy_state = VdevEngyState::STATE_SLEEP;
		// Set vdev state
		*pmem &= ~VDEV_BUSY;
	}
	
	// Power-on: the device is power-on but not ready
	else if (msg.type == SM_Retention::MsgType::POWER_ON) {
		
		// EXTENTION: User defined Recover Procedure

		// Recover from retention
		if (vdev_energy_state == VdevEngyState::STATE_SLEEP) {

			// Restart the disrupted initialization
			if (need_recover && delay_remained == 0) {
				DPRINTF(VirtualDevice, "%s: Restart-Initialization, Need Ticks: %i, Energy: %lf.\n", dev_name, delay_remained, energy_consumed_per_cycle_vdev[VdevEngyState::STATE_ACTIVE] * ticksToCycles(delay_remained)
					);
				schedule(event_init, curTick() + delay_set);
				// Set state: busy again
				*pmem |= VDEV_BUSY;
				// recovered tasks must be actuations
				vdev_energy_state = VdevEngyState::STATE_NORMAL;
				// Alarm cpu to wait
				cpu->virtualDeviceRecover(dev_name, delay_set);
				cpu->virtualDeviceStart(id);
			}

			// Restart the disrupted peripheral task
			else if (need_recover && delay_remained > 0) {
				DPRINTF(VirtualDevice, "%s: Restart-Task, Need Ticks: %i, Energy: %lf.\n", dev_name, delay_remained, energy_consumed_per_cycle_vdev[VdevEngyState::STATE_ACTIVE] * ticksToCycles(delay_remained)
					);
				schedule(event_interrupt, curTick() + delay_remained);
				// Set state: busy again
				*pmem |= VDEV_BUSY;
				// recovered tasks must be actuations
				vdev_energy_state = VdevEngyState::STATE_ACTIVE;
				// Alarm cpu to wait
				cpu->virtualDeviceRecover(dev_name, delay_remained);
				cpu->virtualDeviceStart(id);
				// reset delay_remained
				delay_remained = 0;
			}

			// otherwise, vdev is idle before retention, reset the state to STATE_NORMAL
			else {
				vdev_energy_state = VdevEngyState::STATE_NORMAL;
			}
		}

		// Recover from power off
		else if (vdev_energy_state == VdevEngyState::STATE_POWER_OFF) {
			// restart from the first start: wait for CPU invoking.
		}
	}

	// EXTENTION: add behaviors for your energy state machine.
	//else if (msg.type == YourMsgType){}

	else 
	{
		DPRINTF(EnergyMgmt, "Unrecognized MsgType!\n");
		return 0;
	}

	// Succeed in reacting to energy messages.
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
