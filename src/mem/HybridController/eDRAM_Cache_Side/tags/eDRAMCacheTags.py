from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject
from eDRAMCacheReplacementPolicies import *

class eDRAMCacheTagsWithFABlk(SimObject):
    type = 'eDRAMCacheTagsWithFABlk'
    abstract = True
    cxx_header = "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_tags.hh"

    size = Param.MemorySize(Parent.size, "same as parent")

    block_size = Param.Int(Parent.block_size, "same")

    tag_latency = Param.Cycles(Parent.tag_latency, "same")

    policy = Param.eDRAMCacheFAReplacementPolicy(eDRAMCacheFALRU(), "Reply. policy")

class eDRAMCacheFATags(eDRAMCacheTagsWithFABlk):
    type = 'eDRAMCacheFATags'
    cxx_header = "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_fa_tags.hh"
