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

#include <string.h>
#include "vdev/vdev.hh"
#include "engy/state_machine.hh"
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
	vdev_energy_state(VdevEngyState::POWER_OFF),
	event_interrupt(this, false, Event::Virtual_Interrupt)
{
	sprintf(dev_name, "VDEV-%d", id);
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
	// Todo: confirm that MemObj does not need init()
	//MemObject::init();

	if (port.isConnected()) {
		port.sendRangeChange();
	}

	cpu->registerVDev(delay_recover, id);
	DPRINTF(VirtualDevice, "VDEV(%d) name: %s, range: %#lx - %#lx.\n", id, dev_name, range.start(), range.end());

	vdev_energy_state = VdevEngyState::POWER_OFF;
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
	vdev_energy_state = VdevEngyState::ACTIVE; // The virtual device enter/keep in the active status.
	DPRINTF(VirtualDevice, "Virtual device triggers an interrupt.\n");
	if (*pmem & VDEV_CHAOS) {
		DPRINTF(VirtualDevice, "Virtual device already initialized.\n");
		*pmem |= VDEV_READY;
		*pmem &= ~VDEV_CHAOS;
	} else {
		DPRINTF(VirtualDevice, "Virtual device task already completed.\n");	
	}
	finishSuccess();

	/* Tell cpu. */
	cpu->virtualDeviceInterrupt(delay_cpu_interrupt);
	cpu->virtualDeviceEnd(id);
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
					vdev_energy_state = VdevEngyState::ACTIVE; // the virtual device enter/keep in the active status to complete the initialization
					
					/* Set the virtual device to working mode */
					*pmem |= VDEV_BUSY;

					/* Schedule interrupt after init-delay. */
					schedule(event_interrupt, curTick() + delay_set);
					/* Energy consumption. */
					DPRINTF(VirtualDevice, "Initialization, Need Ticks: %i, Cycles: %i, Energy: %lf. %s state: Init\n", 
						delay_set, 
						ticksToCycles(delay_set), 
						energy_consumed_per_cycle_vdev[VdevEngyState::ACTIVE] * ticksToCycles(delay_set),
						dev_name
					);
					cpu->virtualDeviceSet(delay_set);
					cpu->virtualDeviceStart(id);
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
					vdev_energy_state = VdevEngyState::ACTIVE; // The virtual device enter/keep in the active status.
					
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
					DPRINTF(VirtualDevice, "Start working. Need Ticks:%i, Cycles:%i, Energy: %lf, %s state: Active\n", 
						delay_self, 
						ticksToCycles(delay_self), 
						energy_consumed_per_cycle_vdev[VdevEngyState::ACTIVE] * ticksToCycles(delay_self),
						dev_name
					);
					cpu->virtualDeviceSet(delay_set);
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

// Todo: Find out which function calls tick() of vdev?
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
	if (msg.type == SimpleEnergySM::MsgType::POWER_OFF) 
	{
		// Set energy state
		vdev_energy_state = VdevEngyState::POWER_OFF;

		// Reset vdev to be uninitialized.
		*pmem |= VDEV_CHAOS;
		*pmem &= ~VDEV_READY;
		*pmem &= ~VDEV_BUSY;
		*pmem &= ~VDEV_IDLE;
	}
	
	// Power-on: the device is power-on but not ready
	else if (msg.type == SimpleEnergySM::MsgType::POWER_ON)
	{
		// Set energy state
		vdev_energy_state = VdevEngyState::ACCESS;

		// the functional states are not modified.
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
