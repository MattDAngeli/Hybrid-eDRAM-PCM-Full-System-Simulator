from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class PCMSimMemorySystem(SimObject):
    type = 'PCMSimMemorySystem'
    cxx_class = 'PCMSimMemorySystem'
    cxx_header = "mem/HybridController/PCM_Side/PCMSim/Memory_System/pcm_sim_memory_system.hh"

    clock = Param.Clock(Parent.clock, "same as parent")

    size = Param.MemorySize(Parent.size, "same")

    block_size = Param.Int(Parent.block_size, "same")

    cfg_file = Param.String(Parent.cfg_file, "same")
