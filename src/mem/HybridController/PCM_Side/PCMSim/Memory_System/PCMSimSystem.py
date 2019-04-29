from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class PCMSimMemorySystem(SimObject):
    type = 'PCMSimMemorySystem'
    cxx_class = 'PCMSimMemorySystem'
    cxx_header = "mem/HybridController/PCM_Side/PCMSim/Memory_System/pcm_sim_memory_system.hh"

    clock = Param.Clock(Parent.clock, "PCM frequency")

    size = Param.MemorySize(Parent.size, "capacity of eDRAM")

    block_size = Param.Int(Parent.block_size, "cache line size in bytes")

    cfg_file = Param.String(Parent.cfg_file, "PCM config file")
