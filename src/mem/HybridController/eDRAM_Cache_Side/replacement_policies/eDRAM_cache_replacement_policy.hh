#ifndef __eDRAM_CACHE_REPLACEMENT_POLICY_HH__
#define __eDRAM_CACHE_REPLACEMENT_POLICY_HH__

#include "debug/Gem5Hacking.hh"
#include "params/eDRAMCacheFAReplacementPolicy.hh"

#include "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_tags.hh"
#include "sim/sim_object.hh"

class eDRAMCacheTagsWithFABlk;

template<class T>
class eDRAMCacheReplacementPolicy
{
  public:
    eDRAMCacheReplacementPolicy() {}

    virtual void upgrade(T *blk) {}
    virtual void downgrade(T *blk) {}
    virtual T* findVictim(Addr addr) = 0;
};	

// SimObject should always be the first one
// Very interesting!
class eDRAMCacheFAReplacementPolicy
    : public SimObject,
      public eDRAMCacheReplacementPolicy<eDRAMCacheFABlk>
{
  public:
    eDRAMCacheFAReplacementPolicy(eDRAMCacheFAReplacementPolicyParams *params)
        : SimObject(params),
          eDRAMCacheReplacementPolicy()
    {}

    // This is specific to FA
    virtual void policyInit(eDRAMCacheTagsWithFABlk *tags)
    {
        blks = tags->blks;
        head = &(tags->head);
        tail = &(tags->tail);
    }

  protected:
    eDRAMCacheFABlk *blks;

    eDRAMCacheFABlk **head;
    eDRAMCacheFABlk **tail;
};

#endif
