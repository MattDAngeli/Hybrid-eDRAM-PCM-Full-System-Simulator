#ifndef __eDRAM_CACHE_FA_TAGS_HH__
#define __eDRAM_CACHE_FA_TAGS_HH__

#include "debug/Gem5Hacking.hh"
#include "params/eDRAMCacheFATags.hh"

#include "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_tags.hh"

#include <unordered_map>

class eDRAMCacheFATags : public eDRAMCacheTagsWithFABlk
{
  protected:
    // To make block indexing faster, a hash based address mapping is used
    typedef std::unordered_map<Addr, eDRAMCacheFABlk *> TagHash;
    TagHash tagHash;

  public:
    eDRAMCacheFATags(eDRAMCacheFATagsParams *params);
    
    void tagsInit() override;

    Addr extractTag(Addr addr) const override
    {
        return blkAlign(addr);
    }

    eDRAMCacheFABlk* accessBlock(Addr addr) override;
    eDRAMCacheFABlk* findVictim(Addr addr) override;
    void insertBlock(Addr addr, eDRAMCacheFABlk* victim,
                     uint8_t *new_data, uint8_t *old_data) override;
    void invalidate(eDRAMCacheFABlk* victim) override;

    unsigned numOccupiedBlocks() override { return tagHash.size(); }

  protected:
    eDRAMCacheFABlk* findBlock(Addr addr) const;

    Addr regenerateAddr(eDRAMCacheFABlk *blk) const override
    {
        return blk->tag;
    }
};

#endif
