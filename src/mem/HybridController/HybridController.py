from m5.params import *
from m5.proxy import *
from AbstractMemory import *
from eDRAM import *
from PCM import *

class HybridController(AbstractMemory):
    # PCM Configurations
    type = 'HybridController'
    cxx_header = "mem/HybridController/hybrid_controller.hh"
    cpu_side_port = SlavePort("CPU side port")

    eDRAM = Param.eDRAMCache(eDRAMCache(), "The eDRAM Cache")
    main_memory = Param.PCM(PCM(), "PCM (Main Memory)")
