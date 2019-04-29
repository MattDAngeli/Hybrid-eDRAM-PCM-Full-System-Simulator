#ifndef __eDRAM_CACHE_FA_BLK_HH__
#define __eDRAM_CACHE_FA_BLK_HH__

#include "mem/cache/cache_blk.hh"

class eDRAMCacheFABlk : public CacheBlk
{
  public:
    eDRAMCacheFABlk() : CacheBlk(), prev(nullptr), next(nullptr) {}

    void insert(const Addr tag, const unsigned blkSize,
                const uint8_t *_new_data, const uint8_t *_old_data)
    {
        this->tag = tag;
        tickInserted = curTick();
        setValid();

        assert(_new_data != nullptr);
        assert(_old_data != nullptr);
        std::memcpy(new_data, _new_data, blkSize);
        std::memcpy(old_data, _old_data, blkSize);
    }

    void updateNewData(const uint8_t* _new_data, const unsigned blkSize)
    { 
        assert(_new_data != nullptr);
        std::memcpy(new_data, _new_data, blkSize);
    }

    void retrieve(const unsigned blkSize,
                  uint8_t *_new_data, uint8_t *_old_data)
    {
        assert(isValid());
        assert(_new_data != nullptr);
        assert(_old_data != nullptr);
        
	std::memcpy(_new_data, new_data, blkSize);
        std::memcpy(_old_data, old_data, blkSize);
    }

    void invalidate()
    {
        status = 0;
    }
    /*
     * prev and next are determined by the replacement policy. For example,
     * when LRU is used, prev means the previous block in LRU order.
     *
     * prev (recently used), block, next (least recently used)
     * */
    eDRAMCacheFABlk *prev;

    eDRAMCacheFABlk *next;

    uint8_t *new_data;
    uint8_t *old_data;
};

#endif
