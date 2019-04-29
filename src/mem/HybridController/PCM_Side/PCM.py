from m5.params import *
from m5.proxy import *
from MemObject import *

from PCMProxy import *

class PCM(MemObject):
    type = 'PCM'
    cxx_header = "mem/HybridController/PCM_Side/PCM.hh"

    clock = Param.Clock("200MHz", "PCM frequency")

    size = Param.MemorySize("128GB", "capacity of eDRAM")

    block_size = Param.Int(64, "cache line size in bytes")

    cfg_file = Param.String("src/mem/HybridController/PCM_Side/PCMSim/Configs/cfg_files/PCM.cfg", "PCM config file")

    pcm_proxy = Param.PCMSimProxy(PCMSimProxy(), "PCMSim Proxy")
