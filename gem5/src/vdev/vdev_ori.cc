//
// Created by lf-z on 4/24/17.
//

#include "vdev/vdev.hh"
#include "engy/state_machine.hh"
#include "debug/EnergyMgmt.hh"
#include "debug/VirtualDevice.hh"
#include "debug/MemoryAccess.hh"
#include "engy/DVFS.hh"
#include "engy/DFS_LRY.hh"

#include <fstream>

VirtualDevice::DevicePort::DevicePort(const std::string &_name, VirtualDevice *_vdev)
    : SlavePort(_name, _vdev), vdev(_vdev)
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

VirtualDevice::VirtualDevice(const Params *p)
    : MemObject(p),
      id(0),
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
    
    energy_consumed_per_cycle_vdev[0] = p->energy_consumed_per_cycle_vdev[0];
    energy_consumed_per_cycle_vdev[1] = p->energy_consumed_per_cycle_vdev[1];
    energy_consumed_per_cycle_vdev[2] = p->energy_consumed_per_cycle_vdev[2];
}

void
VirtualDevice::init()
{
    MemObject::init();

    if (port.isConnected()) {
        port.sendRangeChange();
    }

    cpu->registerVDev(delay_recover, id);
    DPRINTF(VirtualDevice, "Virtual Device started with range: %#lx - %#lx\n",
            range.start(), range.end());

    execution_state = STATE_POWEROFF;
}

void
VirtualDevice::triggerInterrupt()
{
    /* Todo: add static of finish success. */
    DPRINTF(VirtualDevice, "Virtual device triggers an interrupt.\n");
    execution_state = STATE_IDLE; // The virtual device enter/keep in the active status.
    finishSuccess();
    assert(*pmem & VDEV_WORK);
    /* Change register byte. */
    *pmem |= VDEV_FINISH;
    *pmem &= ~VDEV_WORK;

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
    } else if (pkt->isWrite()) {
        const uint8_t* pkt_addr = pkt->getConstPtr<uint8_t>();
        if (offset == 0) {
            /* Might be a request. */
            if (*pkt_addr & VDEV_SET) {
                /* Request */
                if (*pmem & VDEV_WORK) {
                    /* Request fails because the vdev is working. */
                    DPRINTF(VirtualDevice, "Request discarded!\n");
                } else {
                    /* Request succeeds. */
                    execution_state = STATE_ACTIVE; // The virtual device enter/keep in the active status.
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
                    *pmem |= VDEV_WORK;
                    *pmem &= ~VDEV_FINISH;
                    /* Schedule interrupt. */
                    schedule(event_interrupt, curTick() + delay_set + delay_self);
                    /* Energy consumption. */
                    DPRINTF(VirtualDevice, "Need Ticks:%i, Cycles:%i, Energy: %lf .\n", 
                    	delay_recover + delay_self, 
                    	ticksToCycles(delay_recover + delay_self), 
                    	energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_recover + delay_self)
                    );
                    consumeEnergy(energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_recover + delay_self));
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


//By LiuRuoyang：这个tick()函数是干什么的？似乎它会按时间每tick扣掉一点能量。
//我想我们可以安全的把那行consumeEnergy给注释掉吧。

void 
VirtualDevice::tick()
{
    
    /*switch(execution_state)
    {
        case STATE_POWEROFF:    
            DPRINTF(VirtualDevice, "Tick: state is POWER OFF.\n"); 
            break;
        case STATE_IDLE:             
            DPRINTF(VirtualDevice, "Tick: state is IDLE(SLEEP).\n", execution_state); 
            break;
        case STATE_ACTIVE:         
            DPRINTF(VirtualDevice, "Tick: state is ACTIVE.\n", execution_state); 
            break;
        default:    DPRINTF(VirtualDevice, "Tick: undefined state!\n", ); break;
    }*/
    //DPRINTF(VirtualDevice, "Tick\n");
    /** Energy consumption **/
    //consumeEnergy(energy_consumed_per_cycle_vdev[execution_state] * ticksToCycles(1));

    //schedule(tickEvent, curTick() + 1);
}

//int
//VirtualDevice::handleMsg(const EnergyMsg &msg)
//{
//    DPRINTF(EnergyMgmt, "Device handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
//    switch(msg.type) {
//        case (int) SimpleEnergySM::POWEROFF:
//            /** Vdev shutdown **/
//            execution_state = STATE_POWEROFF;
//            if (*pmem & VDEV_WORK) {
//                /* This should be handled if the device is on a task **/
//                assert(event_interrupt.scheduled());
//                DPRINTF(VirtualDevice, "device power off occurs in the middle of a task at %lu\n", curTick());
//
//                /* Calculate the remaining delay if the device is interruptable */
//                if (is_interruptable)
//                    delay_remained = event_interrupt.when() - curTick();
//                else
//                    delay_remained = delay_set + delay_self;
//                deschedule(event_interrupt);
//            }
//            break;
//        case (int) SimpleEnergySM::POWERON:
//            /** Vdev shutdown **/
//            execution_state = STATE_ACTIVE;
//            if (*pmem & VDEV_WORK) {
//                assert(!event_interrupt.scheduled());
//                DPRINTF(VirtualDevice, "device power on to finish a task at %lu\n", curTick());
//                schedule(event_interrupt, curTick() + delay_remained);
//                /** Energy consumption **/
//                consumeEnergy(energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained));
//            }
//            break;
//        default:
//            return 0;
//    }
//    return 1;
//}

int
VirtualDevice::handleMsg(const EnergyMsg &msg)
{
    DPRINTF(EnergyMgmt, "Device handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
    switch(msg.type) {
    		case (int) DFS_LRY::MsgType::RETENTION_BEG:
    				//进入RETENTION状态，事实上要做的就是POWEROFF要做的事
    				//因为RETENTION和POWEROFF最大的区别只是开机没惩罚而已
    				execution_state = STATE_POWEROFF;
    				if (*pmem & VDEV_WORK) {
                /* This should be handled if the device is on a task **/
                assert(event_interrupt.scheduled());
                DPRINTF(VirtualDevice, "device retention occurs in the middle of a task at %lu\n", curTick());

                /* Calculate the remaining delay*/
                delay_remained = event_interrupt.when() - curTick();
                deschedule(event_interrupt);
            }
    				break;
        case (int) DFS_LRY::MsgType::POWEROFF:
            /** Vdev shutdown **/           
            /* Re-calculate the delay if the device is interruptable */
            if (*pmem & VDEV_WORK) 
            {
            	DPRINTF(VirtualDevice, "device power off occurs in the middle of a task at %lu\n", curTick());
	            if (!is_interruptable)
	            {
	            	delay_remained = delay_set + delay_self;
	            }
         		}
            break;
        case (int) DFS_LRY::MsgType::RETENTION_END:
    				//从RETENTION状态恢复工作
    				execution_state = STATE_IDLE;
    				if (*pmem & VDEV_WORK) 
    				{
    						execution_state = STATE_ACTIVE;
                assert(!event_interrupt.scheduled());
                DPRINTF(VirtualDevice, "device recover from retention to finish a task at %lu\n", curTick());
                schedule(event_interrupt, curTick() + delay_remained);
            }
            break;
        case (int) DFS_LRY::MsgType::POWERON:
            /** Vdev shutdown **/
            execution_state = STATE_IDLE;
            if (*pmem & VDEV_WORK) {
            		execution_state = STATE_ACTIVE;
                assert(!event_interrupt.scheduled());
                DPRINTF(VirtualDevice, "device power on to finish a task at %lu\n", curTick());
                schedule(event_interrupt, curTick() + delay_remained);
                /** Energy consumption **/
                consumeEnergy(energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained));
            }
            break;
        default:
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
