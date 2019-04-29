#include "mem/HybridController/eDRAM_Cache_Side/replacement_policies/eDRAM_cache_replacement_policy.hh"

#include "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_fa_tags.hh"

eDRAMCacheFATags::eDRAMCacheFATags(eDRAMCacheFATagsParams *params)
    : eDRAMCacheTagsWithFABlk(params)
{
    if (!isPowerOf2(blkSize))
        fatal("cache block size (in bytes) `%d' must be a power of two",
              blkSize);
    if (!isPowerOf2(size))
        fatal("Cache Size must be power of 2 for now");

    DPRINTF(Gem5Hacking, "eDRAMCacheFATags is initiated. \n");
    DPRINTF(Gem5Hacking, "eDRAMCacheFATags, block size: %u\n", blkSize);
    DPRINTF(Gem5Hacking, "eDRAMCacheFATags, cache size (MB): %lld\n",
                         size / 1024 / 1024);
    DPRINTF(Gem5Hacking, "eDRAMCacheFATags, num of blocks: %u\n", numBlocks);
    DPRINTF(Gem5Hacking, "eDRAMCacheFATags, tag lookup latency: %d\n", 
                         static_cast<int>(lookupLatency));

    // Initially, all blocks are invalidated
    blks = new eDRAMCacheFABlk[numBlocks];
}

void eDRAMCacheFATags::tagsInit()
{
    head = &(blks[0]);
    head->prev = nullptr;
    head->next = &(blks[1]);
    head->setPosition(0, 0);
    head->new_data = &new_data[0];
    head->old_data = &old_data[0];

    for (unsigned i = 1; i < numBlocks - 1; i++) 
    {
        blks[i].prev = &(blks[i-1]);
        blks[i].next = &(blks[i+1]);
        blks[i].setPosition(0, i);

        blks[i].new_data = &new_data[blkSize * i];
        blks[i].old_data = &old_data[blkSize * i];
    }

    tail = &(blks[numBlocks - 1]);
    tail->prev = &(blks[numBlocks - 2]);
    tail->next = nullptr;
    tail->setPosition(0, numBlocks - 1);
    tail->new_data = &new_data[(numBlocks - 1) * blkSize];
    tail->old_data = &old_data[(numBlocks - 1) * blkSize];
    
    // Initialize replacement policy here
    policy->policyInit(this);
}

eDRAMCacheFABlk* eDRAMCacheFATags::accessBlock(Addr addr)
{
    DPRINTF(Gem5Hacking, "Accessing... \n");
    eDRAMCacheFABlk *blk = findBlock(addr);

    if (blk && blk->isValid())
    {
        policy->upgrade(blk);
    }

    return blk;
}

eDRAMCacheFABlk* eDRAMCacheFATags::findVictim(Addr addr)
{
    DPRINTF(Gem5Hacking, "Finding a victim block for addr %#x\n", addr);
    eDRAMCacheFABlk* victim = policy->findVictim(addr);

    return victim;
}

void eDRAMCacheFATags::invalidate(eDRAMCacheFABlk* victim)
{
    DPRINTF(Gem5Hacking, "Invalidating block %#x\n", victim->tag);
    assert(tagHash.erase(victim->tag));

    victim->invalidate();

    policy->downgrade(victim);
}

void eDRAMCacheFATags::insertBlock(Addr addr, eDRAMCacheFABlk* victim,
                                   uint8_t *new_data, uint8_t *old_data)
{
    DPRINTF(Gem5Hacking, "Inserting a new block for addr %#x\n", addr);
    assert(!victim->isValid());

    victim->insert(extractTag(addr), blkSize, new_data, old_data);

    policy->upgrade(victim);

    tagHash[victim->tag] = victim;
}

eDRAMCacheFABlk* eDRAMCacheFATags::findBlock(Addr addr) const
{
    DPRINTF(Gem5Hacking, "Trying to find a block for addr %#x \n", addr);
    eDRAMCacheFABlk *blk = nullptr;

    Addr tag = extractTag(addr);
    
    auto iter = tagHash.find(tag);
    if (iter != tagHash.end())
    {
        blk = (*iter).second;
    }

    if (blk && blk->isValid())
    {
        assert(blk->tag == tag);
    }

    return blk;
}

eDRAMCacheFATags*
eDRAMCacheFATagsParams::create()
{
    return new eDRAMCacheFATags(this);
}
