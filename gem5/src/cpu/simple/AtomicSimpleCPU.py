# Copyright (c) 2012 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2007 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Nathan Binkert

from m5.params import *
from BaseSimpleCPU import BaseSimpleCPU
from SimPoint import SimPoint

class AtomicSimpleCPU(BaseSimpleCPU):
    """Simple CPU model executing a configurable number of
    instructions per cycle. This model uses the simplified 'atomic'
    memory mode."""

    type = 'AtomicSimpleCPU'
    cxx_header = "cpu/simple/atomic.hh"

    @classmethod
    def memory_mode(cls):
        return 'atomic'

    @classmethod
    def support_take_over(cls):
        return True

    width = Param.Int(1, "CPU width")
    simulate_data_stalls = Param.Bool(False, "Simulate dcache stall cycles")
    simulate_inst_stalls = Param.Bool(False, "Simulate icache stall cycles")
    
    fastmem = Param.Bool(False, "Access memory directly")


    energy_consumed_per_cycle_5 = Param.Float(2.25, "Energy consumed per cycle in Freq Level 5")
    energy_consumed_per_cycle_4 = Param.Float(1.5, "Energy consumed per cycle in Freq Level 4")
    energy_consumed_per_cycle_3 = Param.Float(1, "Energy consumed per cycle in Freq Level 3")
    energy_consumed_per_cycle_2 = Param.Float(0.5, "Energy consumed per cycle in Freq Level 2")
    energy_consumed_per_cycle_1 = Param.Float(0.25, "Energy consumed per cycle in Freq Level 1")
		
    energy_consumed_poweron = Param.Float(1, "Energy consumed when the system powers on again")
    clockPeriod_to_poweron = Param.Int(1, "Cycles it takes for powering on the system again")
    
    clock_mult_5 = Param.Float(1/2.25, "Clock period multiplier in Freq Level 5")
    clock_mult_4 = Param.Float(1/1.5, "Clock period multiplier in Freq Level 4")
    clock_mult_3 = Param.Float(1, "Clock period multiplier in Freq Level 3")
    clock_mult_2 = Param.Float(1/0.5, "Clock period multiplier in Freq Level 2")
    clock_mult_1 = Param.Float(1/0.25, "Clock period multiplier in Freq Level 1")
                		
    def addSimPointProbe(self, interval):
        simpoint = SimPoint()
        simpoint.interval = interval
        self.probeListener = simpoint
