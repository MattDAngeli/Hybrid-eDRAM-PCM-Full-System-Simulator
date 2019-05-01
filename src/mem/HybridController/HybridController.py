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

    # Cache line size used in Hybrid system
    block_size = Param.Int(64, "cache line size in bytes")
    
    # eDRAM Cache Configurations
    eDRAM_cache_size = Param.MemorySize("64MB", "capacity of eDRAM")
    eDRAM_cache_write_only_mode = Param.Bool(False, "cache writes only?")
    eDRAM_cache_read_partition = Param.MemorySize("32MB", "How much eDRAM for reads")
    eDRAM_cache_write_partition = Param.MemorySize("32MB", "How much for writes")

    eDRAM_cache_tag_latency = Param.Cycles(20, "tag lookup latency")

    eDRAM_cache_mshr_entries = Param.Int(32, "maximum number mshrs")
    eDRAM_cache_wb_entries = Param.Int(32, "maximum number mshrs")

    # PCM Configurations
    PCM_clock = Param.Clock("200MHz", "PCM frequency")
    PCM_size = Param.MemorySize("128GB", "capacity of eDRAM")
    PCM_cfg_file = Param.String("src/mem/HybridController/PCM_Side/PCMSim/Configs/cfg_files/PCM.cfg", "PCM config file")

    # instantiations
    eDRAM = Param.eDRAMCache(eDRAMCache(), "The eDRAM Cache")
    main_memory = Param.PCM(PCM(), "PCM (Main Memory)")
