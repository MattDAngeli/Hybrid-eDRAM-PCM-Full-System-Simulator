from m5.params import *
from m5.proxy import *
from MemObject import *
from eDRAMCacheTags import *

class eDRAMCache(MemObject):
    type = 'eDRAMCache'
    cxx_header = "mem/HybridController/eDRAM_Cache_Side/eDRAM_cache.hh"

    size = Param.MemorySize("64MB", "capacity of eDRAM")
    block_size = Param.Int(64, "cache line size in bytes")
    write_only = Param.Bool(False, "cache writes only?")

    # Assume same tag latency as LLC
    tag_latency = Param.Cycles(20, "tag lookup latency")

    mshr_entries = Param.Int(32, "maximum number mshrs")
    wb_entries = Param.Int(32, "maximum number mshrs")

    tags = Param.eDRAMCacheTagsWithFABlk(eDRAMCacheFATags(), "tag store")
