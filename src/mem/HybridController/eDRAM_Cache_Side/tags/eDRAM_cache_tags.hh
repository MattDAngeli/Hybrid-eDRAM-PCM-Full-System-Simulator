#ifndef __eDRAM_CACHE_TAGS_HH__
#define __eDRAM_CACHE_TAGS_HH__

#include "base/intmath.hh"
#include "params/eDRAMCacheTagsWithFABlk.hh"

#include "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_fa_blk.hh"
#include "sim/sim_object.hh"

#include <memory>

class eDRAMCacheFAReplacementPolicy;

template<class T>
class eDRAMCacheTags
{
  public:
    // Must be a constructor if there are any const type
    eDRAMCacheTags(unsigned _blkSize, unsigned long long _size,
                   Cycles _lookupLatency)
        : blkSize(_blkSize),
          blkMask(_blkSize - 1),
          size(_size),
          numBlocks(size / blkSize),
          lookupLatency(_lookupLatency),
          new_data(new uint8_t[size]),
          old_data(new uint8_t[size])
    {}

    virtual ~eDRAMCacheTags()
    {
        delete blks;
    }

    unsigned getBlkSize() const { return blkSize; }
    unsigned long long getCacheSize() const { return size; }
    unsigned getNumBlocks() const { return numBlocks; }
    Cycles getLookupLatency() const { return lookupLatency; }

  protected:
    const unsigned blkSize; // cache-line (block) size
    const Addr blkMask;
    const unsigned long long size; // (entire) cache size
    const unsigned numBlocks; // number of blocks in the cache

    const Cycles lookupLatency;

    std::unique_ptr<uint8_t[]>new_data; // Contains all the new data to be written
    std::unique_ptr<uint8_t[]>old_data;

  protected:
    T *blks; // All cache blocks

  public:
    virtual void tagsInit() {}

    virtual Addr extractTag(Addr addr) const = 0;

    virtual T* accessBlock(Addr addr) = 0;

    virtual T* findVictim(Addr addr) = 0;

    virtual void insertBlock(Addr addr, T* victim,
                             uint8_t *new_data, uint8_t *old_data) {}

    virtual void invalidate(T* victim) {}

    virtual unsigned numOccupiedBlocks() = 0; // Only make sense for FA

  protected:
    virtual T* findBlock(Addr addr) const = 0;

    // This is a universal function
    Addr blkAlign(Addr addr) const
    {
        return addr & ~blkMask;
    }

    virtual Addr regenerateAddr(T *blk) const = 0;
};

// SimObject should always be the first one
// Very interesting!
class eDRAMCacheTagsWithFABlk :
    public SimObject,
    public eDRAMCacheTags<eDRAMCacheFABlk>
{
  public:
    eDRAMCacheTagsWithFABlk(eDRAMCacheTagsWithFABlkParams *params)
    : SimObject(params),
      eDRAMCacheTags(params->block_size, params->size, params->tag_latency),
      policy(params->policy)
    {}

  protected:
    eDRAMCacheFABlk *head;
    eDRAMCacheFABlk *tail;

  protected:
    eDRAMCacheFAReplacementPolicy *policy;

  friend class eDRAMCacheFAReplacementPolicy;
};

#endif
