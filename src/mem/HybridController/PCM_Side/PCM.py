from m5.params import *
from m5.proxy import *
from MemObject import *

from PCMProxy import *

class PCM(MemObject):
    type = 'PCM'
    cxx_header = "mem/HybridController/PCM_Side/PCM.hh"

    block_size = Param.Int(Parent.block_size, "same as parent")
    
    clock = Param.Clock(Parent.PCM_clock, "same")
    size = Param.MemorySize(Parent.PCM_size, "same")
    cfg_file = Param.String(Parent.PCM_cfg_file, "same")

    pcm_proxy = Param.PCMSimProxy(PCMSimProxy(), "PCMSim Proxy")
