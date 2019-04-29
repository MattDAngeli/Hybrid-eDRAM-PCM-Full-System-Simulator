from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class eDRAMCacheFAReplacementPolicy(SimObject):
    type = 'eDRAMCacheFAReplacementPolicy'
    abstract = True
    cxx_header = "mem/HybridController/eDRAM_Cache_Side/replacement_policies/eDRAM_cache_replacement_policy.hh"

class eDRAMCacheFALRU(eDRAMCacheFAReplacementPolicy):
    type = 'eDRAMCacheFALRU'
    cxx_class = 'eDRAMCacheFALRU'
    cxx_header = "mem/HybridController/eDRAM_Cache_Side/replacement_policies/eDRAM_cache_fa_lru.hh"
