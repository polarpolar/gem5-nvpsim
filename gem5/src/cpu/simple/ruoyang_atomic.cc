/*
 * Copyright 2014 Google, Inc.
 * Copyright (c) 2012-2013 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2002-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Steve Reinhardt
 */

#include <string.h>
#include "arch/locked_mem.hh"
#include "arch/mmapped_ipr.hh"
#include "arch/utility.hh"
#include "base/bigint.hh"
#include "base/output.hh"
#include "config/the_isa.hh"
#include "cpu/simple/atomic.hh"
#include "cpu/exetrace.hh"
#include "debug/Drain.hh"
#include "debug/ExecFaulting.hh"
#include "debug/SimpleCPU.hh"
#include "debug/EnergyMgmt.hh"
#include "debug/VirtualDevice.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "mem/physical.hh"
#include "params/AtomicSimpleCPU.hh"
#include "sim/faults.hh"
#include "sim/system.hh"
#include "sim/full_system.hh"
#include "engy/state_machine.hh"
#include "engy/two_thres.hh"
#include "engy/DVFS.hh"
#include "engy/DFS_LRY.hh"

using namespace std;
using namespace TheISA;

AtomicSimpleCPU::TickEvent::TickEvent(AtomicSimpleCPU *c)
    : Event(CPU_Tick_Pri), cpu(c)
{
}

void
AtomicSimpleCPU::TickEvent::process()
{
    cpu->tick();
}

const char *
AtomicSimpleCPU::TickEvent::description() const
{
    return "AtomicSimpleCPU tick";
}

void
AtomicSimpleCPU::init()
{
    BaseCPU::init();

    // Initialise the ThreadContext's memory proxies
    tcBase()->initMemProxies(tcBase());

    if (FullSystem && !params()->switched_out) {
        ThreadID size = threadContexts.size();
        for (ThreadID i = 0; i < size; ++i) {
            ThreadContext *tc = threadContexts[i];
            // initialize CPU, including PC
            TheISA::initCPU(tc, tc->contextId());
        }
    }

    // Atomic doesn't do MT right now, so contextId == threadId
    ifetch_req.setThreadContext(_cpuId, 0); // Add thread ID if we add MT
    data_read_req.setThreadContext(_cpuId, 0); // Add thread ID here too
    data_write_req.setThreadContext(_cpuId, 0); // Add thread ID here too
}

AtomicSimpleCPU::AtomicSimpleCPU(AtomicSimpleCPUParams *p)
    : BaseSimpleCPU(p),
    	
    	power_cpu_5(p->power_cpu_5),
      power_cpu_4(p->power_cpu_4),
      power_cpu_3(p->power_cpu_3),
      power_cpu_2(p->power_cpu_2),
      power_cpu_1(p->power_cpu_1),
	  
      power_cpu(p->power_cpu),  
      cycle_restore(p->cycle_restore),
	  
      clock_mult_5(p->clock_mult_5),
      clock_mult_4(p->clock_mult_4),
      clock_mult_3(p->clock_mult_3),
      clock_mult_2(p->clock_mult_2),
      clock_mult_1(p->clock_mult_1),
      
    	tickEvent(this), width(p->width), locked(false),
      simulate_data_stalls(p->simulate_data_stalls),
      simulate_inst_stalls(p->simulate_inst_stalls),
      drain_manager(NULL),
      icachePort(name() + ".icache_port", this),
      dcachePort(name() + ".dcache_port", this),
      fastmem(p->fastmem), dcache_access(false), dcache_latency(0),
      vdev_set(false), vdev_set_latency(0),
      ppCommit(nullptr), power_cpu(0),
      in_interrupt(0)
{
    strcpy(dev_name, "AtomicCPU");
    _status = Idle;
    tick_remain = 0;
}


AtomicSimpleCPU::~AtomicSimpleCPU()
{
    if (tickEvent.scheduled()) {
        deschedule(tickEvent);
    }
}

unsigned int
AtomicSimpleCPU::drain(DrainManager *dm)
{
    assert(!drain_manager);
    if (switchedOut())
        return 0;

    if (!isDrained()) {
        DPRINTF(Drain, "Requesting drain: %s\n", pcState());
        drain_manager = dm;
        return 1;
    } else {
        if (tickEvent.scheduled())
            deschedule(tickEvent);

        DPRINTF(Drain, "Not executing microcode, no need to drain.\n");
        return 0;
    }
}

void
AtomicSimpleCPU::drainResume()
{
    assert(!tickEvent.scheduled());
    assert(!drain_manager);
    if (switchedOut())
        return;

    DPRINTF(SimpleCPU, "Resume\n");
    verifyMemoryMode();

    assert(!threadContexts.empty());
    if (threadContexts.size() > 1)
        fatal("The atomic CPU only supports one thread.\n");

    if (thread->status() == ThreadContext::Active) {
        schedule(tickEvent, nextCycle());
        _status = BaseSimpleCPU::Running;
        notIdleFraction = 1;
    } else {
        _status = BaseSimpleCPU::Idle;
        notIdleFraction = 0;
    }
}

bool
AtomicSimpleCPU::tryCompleteDrain()
{
    if (!drain_manager)
        return false;

    DPRINTF(Drain, "tryCompleteDrain: %s\n", pcState());
    if (!isDrained())
        return false;

    DPRINTF(Drain, "CPU done draining, processing drain event\n");
    drain_manager->signalDrainDone();
    drain_manager = NULL;

    return true;
}


void
AtomicSimpleCPU::switchOut()
{
    BaseSimpleCPU::switchOut();

    assert(!tickEvent.scheduled());
    assert(_status == BaseSimpleCPU::Running || _status == Idle);
    assert(isDrained());
}


void
AtomicSimpleCPU::takeOverFrom(BaseCPU *oldCPU)
{
    BaseSimpleCPU::takeOverFrom(oldCPU);

    // The tick event should have been descheduled by drain()
    assert(!tickEvent.scheduled());

    ifetch_req.setThreadContext(_cpuId, 0); // Add thread ID if we add MT
    data_read_req.setThreadContext(_cpuId, 0); // Add thread ID here too
    data_write_req.setThreadContext(_cpuId, 0); // Add thread ID here too
}

void
AtomicSimpleCPU::verifyMemoryMode() const
{
    if (!system->isAtomicMode()) {
        fatal("The atomic CPU requires the memory system to be in "
              "'atomic' mode.\n");
    }
}

void
AtomicSimpleCPU::activateContext(ThreadID thread_num)
{
    DPRINTF(SimpleCPU, "ActivateContext %d\n", thread_num);

    assert(thread_num == 0);
    assert(thread);

    assert(_status == Idle);
    assert(!tickEvent.scheduled());

    notIdleFraction = 1;
    Cycles delta = ticksToCycles(thread->lastActivate - thread->lastSuspend);
    numCycles += delta;
    ppCycles->notify(delta);

    //Make sure ticks are still on multiples of cycles
    schedule(tickEvent, clockEdge(Cycles(0)));
    _status = BaseSimpleCPU::Running;
}


void
AtomicSimpleCPU::suspendContext(ThreadID thread_num)
{
    DPRINTF(SimpleCPU, "SuspendContext %d\n", thread_num);

    assert(thread_num == 0);
    assert(thread);

    if (_status == Idle)
        return;

    assert(_status == BaseSimpleCPU::Running);

    // tick event may not be scheduled if this gets called from inside
    // an instruction's execution, e.g. "quiesce"
    if (tickEvent.scheduled())
        deschedule(tickEvent);

    notIdleFraction = 0;
    _status = Idle;
}


Tick
AtomicSimpleCPU::AtomicCPUDPort::recvAtomicSnoop(PacketPtr pkt)
{
    DPRINTF(SimpleCPU, "received snoop pkt for addr:%#x %s\n", pkt->getAddr(),
            pkt->cmdString());

    // X86 ISA: Snooping an invalidation for monitor/mwait
    AtomicSimpleCPU *cpu = (AtomicSimpleCPU *)(&owner);
    if(cpu->getAddrMonitor()->doMonitor(pkt)) {
        cpu->wakeup();
    }

    // if snoop invalidates, release any associated locks
    if (pkt->isInvalidate()) {
        DPRINTF(SimpleCPU, "received invalidation for addr:%#x\n",
                pkt->getAddr());
        TheISA::handleLockedSnoop(cpu->thread, pkt, cacheBlockMask);
    }

    return 0;
}

void
AtomicSimpleCPU::AtomicCPUDPort::recvFunctionalSnoop(PacketPtr pkt)
{
    DPRINTF(SimpleCPU, "received snoop pkt for addr:%#x %s\n", pkt->getAddr(),
            pkt->cmdString());

    // X86 ISA: Snooping an invalidation for monitor/mwait
    AtomicSimpleCPU *cpu = (AtomicSimpleCPU *)(&owner);
    if(cpu->getAddrMonitor()->doMonitor(pkt)) {
        cpu->wakeup();
    }

    // if snoop invalidates, release any associated locks
    if (pkt->isInvalidate()) {
        DPRINTF(SimpleCPU, "received invalidation for addr:%#x\n",
                pkt->getAddr());
        TheISA::handleLockedSnoop(cpu->thread, pkt, cacheBlockMask);
    }
}

Fault
AtomicSimpleCPU::readMem(Addr addr, uint8_t * data,
                         unsigned size, unsigned flags)
{
    // use the CPU's statically allocated read request and packet objects
    Request *req = &data_read_req;

    if (traceData)
        traceData->setMem(addr, size, flags);

    //The size of the data we're trying to read.
    int fullSize = size;

    //The address of the second part of this access if it needs to be split
    //across a cache line boundary.
    Addr secondAddr = roundDown(addr + size - 1, cacheLineSize());

    if (secondAddr > addr)
        size = secondAddr - addr;

    dcache_latency = 0;

    req->taskId(taskId());
    while (1) {
        req->setVirt(0, addr, size, flags, dataMasterId(), thread->pcState().instAddr());

        // translate to physical address
        Fault fault = thread->dtb->translateAtomic(req, tc, BaseTLB::Read);

        // Now do the access.
        if (fault == NoFault && !req->getFlags().isSet(Request::NO_ACCESS)) {
            Packet pkt(req, Packet::makeReadCmd(req));
            pkt.dataStatic(data);

            if (req->isMmappedIpr())
                dcache_latency += TheISA::handleIprRead(thread->getTC(), &pkt);
            else {
                if (fastmem && system->isMemAddr(pkt.getAddr()))
                    system->getPhysMem().access(&pkt);
                else
                    dcache_latency += dcachePort.sendAtomic(&pkt);
            }
            dcache_access = true;

            assert(!pkt.isError());

            if (req->isLLSC()) {
                TheISA::handleLockedRead(thread, req);
            }
        }

        //If there's a fault, return it
        if (fault != NoFault) {
            if (req->isPrefetch()) {
                return NoFault;
            } else {
                return fault;
            }
        }

        //If we don't need to access a second cache line, stop now.
        if (secondAddr <= addr)
        {
            if (req->isLockedRMW() && fault == NoFault) {
                assert(!locked);
                locked = true;
            }
            return fault;
        }

        /*
         * Set up for accessing the second cache line.
         */

        //Move the pointer we're reading into to the correct location.
        data += size;
        //Adjust the size to get the remaining bytes.
        size = addr + fullSize - secondAddr;
        //And access the right address.
        addr = secondAddr;
    }
}


Fault
AtomicSimpleCPU::writeMem(uint8_t *data, unsigned size,
                          Addr addr, unsigned flags, uint64_t *res)
{

    static uint8_t zero_array[64] = {};

    if (data == NULL) {
        assert(size <= 64);
        assert(flags & Request::CACHE_BLOCK_ZERO);
        // This must be a cache block cleaning request
        data = zero_array;
    }

    // use the CPU's statically allocated write request and packet objects
    Request *req = &data_write_req;

    if (traceData)
        traceData->setMem(addr, size, flags);

    //The size of the data we're trying to read.
    int fullSize = size;

    //The address of the second part of this access if it needs to be split
    //across a cache line boundary.
    Addr secondAddr = roundDown(addr + size - 1, cacheLineSize());

    if(secondAddr > addr)
        size = secondAddr - addr;

    dcache_latency = 0;

    req->taskId(taskId());
    while(1) {
        req->setVirt(0, addr, size, flags, dataMasterId(), thread->pcState().instAddr());

        // translate to physical address
        Fault fault = thread->dtb->translateAtomic(req, tc, BaseTLB::Write);

        // Now do the access.
        if (fault == NoFault) {
            MemCmd cmd = MemCmd::WriteReq; // default
            bool do_access = true;  // flag to suppress cache access

            if (req->isLLSC()) {
                cmd = MemCmd::StoreCondReq;
                do_access = TheISA::handleLockedWrite(thread, req, dcachePort.cacheBlockMask);
            } else if (req->isSwap()) {
                cmd = MemCmd::SwapReq;
                if (req->isCondSwap()) {
                    assert(res);
                    req->setExtraData(*res);
                }
            }

            if (do_access && !req->getFlags().isSet(Request::NO_ACCESS)) {
                Packet pkt = Packet(req, cmd);
                pkt.dataStatic(data);

                if (req->isMmappedIpr()) {
                    dcache_latency +=
                        TheISA::handleIprWrite(thread->getTC(), &pkt);
                } else {
                    if (fastmem && system->isMemAddr(pkt.getAddr()))
                        system->getPhysMem().access(&pkt);
                    else
                        dcache_latency += dcachePort.sendAtomic(&pkt);
                }
                dcache_access = true;
                assert(!pkt.isError());

                if (req->isSwap()) {
                    assert(res);
                    memcpy(res, pkt.getConstPtr<uint8_t>(), fullSize);
                }
            }

            if (res && !req->isSwap()) {
                *res = req->getExtraData();
            }
        }

        //If there's a fault or we don't need to access a second cache line,
        //stop now.
        if (fault != NoFault || secondAddr <= addr)
        {
            if (req->isLockedRMW() && fault == NoFault) {
                assert(locked);
                locked = false;
            }
            if (fault != NoFault && req->isPrefetch()) {
                return NoFault;
            } else {
                return fault;
            }
        }

        /*
         * Set up for accessing the second cache line.
         */

        //Move the pointer we're reading into to the correct location.
        data += size;
        //Adjust the size to get the remaining bytes.
        size = addr + fullSize - secondAddr;
        //And access the right address.
        addr = secondAddr;
    }
}


void
AtomicSimpleCPU::tick()
{
    DPRINTF(SimpleCPU, "Tick\n");

    in_interrupt = 0;

    Tick latency = 0;

    for (int i = 0; i < width || locked; ++i) {
        numCycles++;
        ppCycles->notify(1);

        if (!curStaticInst || !curStaticInst->isDelayedCommit()) {
            checkForInterrupts();
            checkPcEventQueue();
        }

        // We must have just got suspended by a PC event
        if (_status == Idle) {
            tryCompleteDrain();
            return;
        }

        Fault fault = NoFault;

        TheISA::PCState pcState = thread->pcState();

        bool needToFetch = !isRomMicroPC(pcState.microPC()) &&
                           !curMacroStaticInst;
        if (needToFetch) {
            ifetch_req.taskId(taskId());
            setupFetchRequest(&ifetch_req);
            fault = thread->itb->translateAtomic(&ifetch_req, tc,
                                                 BaseTLB::Execute);
        }

        if (fault == NoFault) {
            Tick icache_latency = 0;
            bool icache_access = false;
            dcache_access = false; // assume no dcache access

            if (needToFetch) {
                // This is commented out because the decoder would act like
                // a tiny cache otherwise. It wouldn't be flushed when needed
                // like the I cache. It should be flushed, and when that works
                // this code should be uncommented.
                //Fetch more instruction memory if necessary
                //if(decoder.needMoreBytes())
                //{
                    icache_access = true;
                    Packet ifetch_pkt = Packet(&ifetch_req, MemCmd::ReadReq);
                    ifetch_pkt.dataStatic(&inst);

                    if (fastmem && system->isMemAddr(ifetch_pkt.getAddr()))
                        system->getPhysMem().access(&ifetch_pkt);
                    else
                        icache_latency = icachePort.sendAtomic(&ifetch_pkt);

                    assert(!ifetch_pkt.isError());

                    // ifetch_req is initialized to read the instruction directly
                    // into the CPU object's inst field.
                //}
            }

            preExecute();

            if (curStaticInst) {
                fault = curStaticInst->execute(this, traceData);

                // keep an instruction count
                if (fault == NoFault) {
                    countInst();
                    ppCommit->notify(std::make_pair(thread, curStaticInst));
                }
                else if (traceData && !DTRACE(ExecFaulting)) {
                    delete traceData;
                    traceData = NULL;
                }

                postExecute();
            }

            // @todo remove me after debugging with legion done
            if (curStaticInst && (!curStaticInst->isMicroop() ||
                        curStaticInst->isFirstMicroop()))
                instCnt++;

            Tick stall_ticks = 0;
            if (simulate_inst_stalls && icache_access)
                stall_ticks += icache_latency;

            if (simulate_data_stalls && dcache_access)
                stall_ticks += dcache_latency;

            if (vdev_set) {
                vdev_set = 0;
                stall_ticks += vdev_set_latency;
            }

            if (stall_ticks) {
                // the atomic cpu does its accounting in ticks, so
                // keep counting in ticks but round to the clock
                // period
                latency += divCeil(stall_ticks, clockPeriod()) *
                    clockPeriod();
            }

        }
        if(fault != NoFault || !stayAtPC)
            advancePC(fault);
    }

    if (tryCompleteDrain())
        return;

    // instruction takes at least one cycle
    if (latency < clockPeriod())
        latency = clockPeriod();

    consumeEnergy(dev_name, power_cpu * ticksToCycles(latency));

    if (_status != Idle)
        schedule(tickEvent, curTick() + latency);
}

void
AtomicSimpleCPU::regProbePoints()
{
    BaseCPU::regProbePoints();

    ppCommit = new ProbePointArg<pair<SimpleThread*, const StaticInstPtr>>
                                (getProbeManager(), "Commit");
}

void
AtomicSimpleCPU::printAddr(Addr a)
{
    dcachePort.printAddr(a);
}


// two-threshold handler
/*int
AtomicSimpleCPU::handleMsg(const EnergyMsg &msg)
{
    int rlt = 1;
    Tick lat = 0;
    DPRINTF(EnergyMgmt, "[AtomicSimpleCPU-Two-Thres] handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
    switch(msg.type){
        case (int) TwoThresSM::MsgType::POWEROFF:
            lat = tickEvent.when() - curTick();
            if (in_interrupt)
                tick_remain = lat + clockPeriod() - lat % clockPeriod();
            else
                tick_remain = 0;
            deschedule(tickEvent);
            break;
        case (int) TwoThresSM::MsgType::POWERON:
            consumeEnergy(power_cpu * ticksToCycles(tick_remain + BaseCPU::getTotalLat()));
            schedule(tickEvent, curTick() + tick_remain + BaseCPU::getTotalLat());
            break;
        default:
            rlt = 0;
    }
    return rlt;
}*/

int
AtomicSimpleCPU::handleMsg(const EnergyMsg &msg) {
    return 0;
}

/*
int
AtomicSimpleCPU::handleMsg(const EnergyMsg &msg)
{
    int rlt = 1;
    Tick lat = 0;
    //const int CPU_Power = 0.160;

    DPRINTF(EnergyMgmt, "[AtomicSimpleCPU-DVFS] handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
    switch(msg.type){
        case (int) DVFSSM::MsgType::POWEROFF:
            lat = tickEvent.when() - curTick();
            if (in_interrupt)
                tick_remain = lat + clockPeriod() - lat % clockPeriod();
            else
                tick_remain = 0;
            deschedule(tickEvent);
            break;
        case (int) DVFSSM::MsgType::DVFS_LOW:
            power_cpu *= 0.25;//DVFSSM::rateLow^2 * CPU_Power * 1;
            // frequency
            consumeEnergy(power_cpu * ticksToCycles(0 + BaseCPU::getTotalLat()));
            schedule(tickEvent, curTick() + 0 + BaseCPU::getTotalLat());
            break;
        case (int) DVFSSM::MsgType::DVFS_MIDDLE:
            power_cpu *= 1;//DVFSSM::rateMid^2 * CPU_Power * 1;
            // frequency
            //consumeEnergy(power_cpu * ticksToCycles(tick_remain + BaseCPU::getTotalLat()));
            //schedule(tickEvent, curTick() + tick_remain + BaseCPU::getTotalLat());
            break;
        case (int) DVFSSM::MsgType::DVFS_HIGH:
            power_cpu *= 2.25;//DVFSSM::rateHigh^2 * CPU_Power * 1;
            // frequency
            //consumeEnergy(power_cpu * ticksToCycles(tick_remain + BaseCPU::getTotalLat()));
            //schedule(tickEvent, curTick() + tick_remain + BaseCPU::getTotalLat());
            break;
        default:
            rlt = 0;
    }
    return rlt;
}
*/

/*
//Msg handle for DFS_LRY
int
AtomicSimpleCPU::handleMsg(const EnergyMsg &msg)
{
	  int rlt = 1;
    Tick lat = 0;
    //const int CPU_Power = 0.160;

    DPRINTF(EnergyMgmt, "[AtomicSimpleCPU-DFS_LRY] handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
    switch(msg.type){
    		case (int) DFS_LRY::MsgType::RETENTION_BEG:
    				//进入RETENTION状态，事实上要做的就是POWEROFF要做的事。
    				//因为RETENTION和POWEROFF最大的区别只是开机没惩罚而已。
    				lat = tickEvent.when() - curTick();           
            tick_remain = lat + clockPeriod() - lat % clockPeriod();
            deschedule(tickEvent);
    				break;
    				
        case (int) DFS_LRY::MsgType::POWEROFF:
        		//关机
        		//该做的事实际上大部分在进入RETENTION时已经做了
        		
        		//仅有的一次例外是整个simulate刚开始时，会经历一次poweroff，这次是不会先retention的
        		if(tickEvent.scheduled())
						{
							DPRINTF(EnergyMgmt, "[AtomicSimpleCPU-DFS_LRY] handleMsg called at %lu, msg.type=%d. This is the initialization poweroff.\n", curTick(), msg.type);
							lat = tickEvent.when() - curTick();           
            	tick_remain = lat + clockPeriod() - lat % clockPeriod();
            	deschedule(tickEvent);
						} 
						       		
        		if (!in_interrupt)
        		{
        			tick_remain = 0;
        		}        			
            break;
            
        case (int) DFS_LRY::MsgType::RETENTION_END:
    				//从RETENTION状态恢复工作，进入Freq Lv1
    				//但是和POWERON不同的是没有额外的开机惩罚
    				//调整频率
            clkmult = clock_mult_1;
        	  //调整耗能
            power_cpu = power_cpu_1;
            //开机
            schedule(tickEvent, curTick() + tick_remain);                      
    				break;  
    				  
        case (int) DFS_LRY::MsgType::POWERON:
        	  //开机，进入Freq Lv1
        	  //调整频率
            clkmult = clock_mult_1;
        	  //调整耗能
            power_cpu = power_cpu_1;
            //开机惩罚耗能
            consumeEnergy(
                dev_name,
                power_cpu + power_cpu * ticksToCycles(
                tick_remain + BaseCPU::getTotalLat()
              )
            );
            //开机惩罚延时
            schedule(tickEvent, 
              curTick() + 
              tick_remain + 
              BaseCPU::getTotalLat() + 
              clockPeriod() * cycle_restore
            );                      
            break;
            
        case (int) DFS_LRY::MsgType::FREQ2to1:
        	  //进入能耗等级：1
        	  //调整频率
            clkmult = clock_mult_1;
        	  //调整耗能
            power_cpu = power_cpu_1;             
            break; 
 
        case (int) DFS_LRY::MsgType::FREQ1to2:
        case (int) DFS_LRY::MsgType::FREQ3to2:
        	  //进入能耗等级：2
        	  //调整频率
            clkmult = clock_mult_2;
        	  //调整耗能
            power_cpu = power_cpu_2;             
            break;
         
        case (int) DFS_LRY::MsgType::FREQ2to3:   
        case (int) DFS_LRY::MsgType::FREQ4to3:
        	  //进入能耗等级：3
        	  //调整频率
            clkmult = clock_mult_3;
        	  //调整耗能
            power_cpu = power_cpu_3;             
            break;
            
        case (int) DFS_LRY::MsgType::FREQ3to4:
        case (int) DFS_LRY::MsgType::FREQ5to4:
        	  //进入能耗等级：4
        	  //调整频率
            clkmult = clock_mult_4;
        	  //调整耗能
            power_cpu = power_cpu_4;             
            break;
            
        case (int) DFS_LRY::MsgType::FREQ4to5:
        	  //进入能耗等级：5
        	  //调整频率
            clkmult = clock_mult_5;
        	  //调整耗能
            power_cpu = power_cpu_5;             
            break;                                    
        default:
            rlt = 0;
    }
    return rlt;
}*/

int
AtomicSimpleCPU::virtualDeviceDelay(Tick tick)
{
    int rlt = 1;
    Tick time = tickEvent.when();
    if (tick % clockPeriod())
        tick += clockPeriod() - tick % clockPeriod();
    time += tick;
    reschedule(tickEvent, time);
    return rlt;
}

int
AtomicSimpleCPU::virtualDeviceInterrupt(Tick tick)
{
    in_interrupt = 1;
    DPRINTF(VirtualDevice, "AtomicCPU virtualDeviceInterrupt vdev_int_latency = %#lu\n", tick);
    return virtualDeviceDelay(tick);
}

int AtomicSimpleCPU::virtualDeviceSet(Tick tick)
{
    int rlt = 1;
    vdev_set = 1;
    vdev_set_latency = tick;
    DPRINTF(VirtualDevice, "AtomicCPU virtualDeviceSet vdev_set_latency = %#lu\n", vdev_set_latency);
    return rlt;
}

////////////////////////////////////////////////////////////////////////
//
//  AtomicSimpleCPU Simulation Object
//
AtomicSimpleCPU *
AtomicSimpleCPUParams::create()
{
    numThreads = 1;
    if (!FullSystem && workload.size() != 1)
        panic("only one workload allowed");
    return new AtomicSimpleCPU(this);
}
