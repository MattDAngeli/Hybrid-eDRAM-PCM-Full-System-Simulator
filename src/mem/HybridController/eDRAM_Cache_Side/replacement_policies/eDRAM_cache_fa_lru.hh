#ifndef __eDRAM_CACHE_FA_LRU_HH__
#define __eDRAM_CACHE_FA_LRU_HH__

#include "debug/Gem5Hacking.hh"
#include "params/eDRAMCacheFALRU.hh"

#include "mem/HybridController/eDRAM_Cache_Side/replacement_policies/eDRAM_cache_replacement_policy.hh"
#include "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_tags.hh"

class eDRAMCacheFALRU : public eDRAMCacheFAReplacementPolicy
{
  public:
    eDRAMCacheFALRU(eDRAMCacheFALRUParams *params);

    void upgrade(eDRAMCacheFABlk *blk) override;
    void downgrade(eDRAMCacheFABlk *blk) override;
    eDRAMCacheFABlk* findVictim(Addr addr) override;
};

#endif
