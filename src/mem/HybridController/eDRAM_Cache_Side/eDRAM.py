from m5.params import *
from m5.proxy import *
from MemObject import *
from eDRAMCacheTags import *

class eDRAMCache(MemObject):
    type = 'eDRAMCache'
    cxx_header = "mem/HybridController/eDRAM_Cache_Side/eDRAM_cache.hh"

    block_size = Param.Int(Parent.block_size, "same as parent")
    
    size = Param.MemorySize(Parent.eDRAM_cache_size, "same")
    write_only = Param.Bool(Parent.eDRAM_cache_write_only_mode, "same")
    read_part = Param.MemorySize(Parent.eDRAM_cache_read_partition, "same")
    write_part = Param.MemorySize(Parent.eDRAM_cache_write_partition, "same")

    tag_latency = Param.Cycles(Parent.eDRAM_cache_tag_latency, "same")

    mshr_entries = Param.Int(Parent.eDRAM_cache_mshr_entries, "same")
    wb_entries = Param.Int(Parent.eDRAM_cache_wb_entries, "same")

    tags = Param.eDRAMCacheTagsWithFABlk(eDRAMCacheFATags(), "tag store")
