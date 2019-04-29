#include "mem/HybridController/eDRAM_Cache_Side/eDRAM_cache.hh"

#include "mem/HybridController/hybrid_controller.hh"
#include "mem/HybridController/PCM_Side/PCM.hh"

eDRAMCache::eDRAMCache(const eDRAMCacheParams *params)
    : MemObject(params),
      tick_event(this),
      mshr_cb_func(std::bind(&eDRAMCache::MSHRComplete, this,
                          std::placeholders::_1)),
      wb_cb_func(std::bind(&eDRAMCache::WBComplete, this,
                          std::placeholders::_1)),
      blkSize(params->block_size),
      tags(params->tags),
      mshrs(new eDRAMSimulator::Deferred_Set(params->mshr_entries, blkSize)),
      wb_queue(new eDRAMSimulator::Deferred_Set(params->wb_entries, blkSize))
{
    ticks_per_clk = clockPeriod();
    DPRINTF(Gem5Hacking, "eDRAM clock period: %lld\n",
                         static_cast<unsigned long long>(ticks_per_clk));

    tag_delay = cyclesToTicks(tags->getLookupLatency());

    // Initialize tags here
    tags->tagsInit();
}

void eDRAMCache::startup()
{
    schedule(tick_event, clockEdge());
}

void eDRAMCache::processTickEvent()
{
    sendDeferredReq();
    schedule(tick_event, curTick() + ticks_per_clk);
}

void eDRAMCache::sendDeferredReq()
{
    Addr mshr_entry = MaxAddr;
    bool mshr_entry_valid = mshrs->getEntry(mshr_entry);

    Addr wb_entry = MaxAddr;
    bool wb_entry_valid = wb_queue->getEntry(wb_entry);

    if (wb_entry_valid && (wb_queue->isFull() || !mshr_entry_valid))
    {
        assert(wb_entry != MaxAddr);
        sendWBReq(wb_entry);
    }
    else if (mshr_entry_valid)
    {
        assert(mshr_entry != MaxAddr);
        sendMSHRReq(mshr_entry);
    }
}

bool eDRAMCache::access(PacketPtr pkt)
{
    eDRAMCacheFABlk *
    blk = tags->accessBlock(pkt->getAddr());

    if (blk && blk->isValid())
    {
        if (pkt->isWrite())
        {
            num_of_write_hits++;
            blk->updateNewData(pkt->getConstPtr<uint8_t>(), blkSize);
        }
        else
        {
            num_of_read_hits++;
        }
        hybridC->accessAndRespond(pkt);
        return true;
    }

    // we should consider the wb queue as well.
    bool in_wb_queue = wb_queue->isInQueue(pkt->getAddr());
    if (in_wb_queue)
    {
        Request req(pkt->getAddr(),
                    Request::Request_Type::WRITE, blkSize, wb_cb_func);
        auto ite = wb_queue->data_entries.find(pkt->getAddr());
        assert(ite != wb_queue->data_entries.end());
        eDRAMSimulator::Data_Entry &entry = ite->second;
        // retrieve the data
        entry.retrieve(req.new_data.get(), req.old_data.get());
        if (pkt->isWrite())
        {
            std::memcpy(req.new_data.get(), pkt->getConstPtr<uint8_t>(), blkSize);
        }
        num_of_wb_hits++;
        allocateBlock(req); // re-allocate this block
        wb_queue->deAllocate(req.addr, false);

	hybridC->accessAndRespond(pkt);
        return true;
    }

    if (pkt->isWrite() && !blocked())
    {
        assert(!mshrs->isFull());
        assert(!wb_queue->isFull());

        // For a write miss, send a read to PCM
        Addr target = tags->extractTag(pkt->getAddr());
        mshrs->allocate(target);
        DPRINTF(Gem5Hacking, "eDRAM detected a write miss. Allocating MSHR for "
                             "block %#x\n", target);
        
	// Record data information for this write packet
        auto ite = mshrs->data_entries.find(target);
        assert(ite != mshrs->data_entries.end());
        eDRAMSimulator::Data_Entry &entry = ite->second;
        hybridC->dataRetrieval(pkt,
                               entry.new_data.get(),
                               entry.old_data.get());

        hybridC->accessAndRespond(pkt);

	return true;
    }

    return false;
}

void eDRAMCache::sendMSHRReq(Addr addr)
{
    Request req(addr, Request::Request_Type::READ, blkSize, mshr_cb_func);
    // We need to record data now and later write to victim block in 
    // callback function
    auto ite = mshrs->data_entries.find(addr);
    assert(ite != mshrs->data_entries.end());
    eDRAMSimulator::Data_Entry &entry = ite->second;
    entry.retrieve(req.new_data.get(), req.old_data.get());

    if (pcm->access(req))
    {
        DPRINTF(Gem5Hacking, "eDRAM sent an MSHR request for block %#x\n", addr);
        mshrs->entryOnBoard(addr);
    }
}

void eDRAMCache::sendWBReq(Addr addr)
{
    Request req(addr, Request::Request_Type::WRITE, blkSize, wb_cb_func);
    // Retrieve data from wb_queue
    auto ite = wb_queue->data_entries.find(addr);
    assert(ite != wb_queue->data_entries.end());
    eDRAMSimulator::Data_Entry &entry = ite->second;
    entry.retrieve(req.new_data.get(), req.old_data.get());

    if (pcm->access(req))
    {
        DPRINTF(Gem5Hacking, "eDRAM sent an WB request for block %#x\n", addr);
        wb_queue->entryOnBoard(addr);
    }
}

void eDRAMCache::MSHRComplete(Request& req)
{
    DPRINTF(Gem5Hacking, "A MSHR request for block %#x has been completed. \n",
                         req.addr);

    // Step one: Allocate eDRAM block
    allocateBlock(req);

    // Step two: De-allocate MSHR entry
    mshrs->deAllocate(req.addr);
    DPRINTF(Gem5Hacking, "MSHR entry for block %#x has been de-allocated. \n",
                         req.addr);
}

void eDRAMCache::WBComplete(Request& req)
{
    DPRINTF(Gem5Hacking, "A WB request for block %#x has been completed. \n",
                         req.addr);

    // Step one: De-allocate wb entry
    wb_queue->deAllocate(req.addr);
    DPRINTF(Gem5Hacking, "WB entry for block %#x has been de-allocated. \n",
                         req.addr);
}

void eDRAMCache::allocateBlock(Request &req)
{
    DPRINTF(Gem5Hacking, "eDRAM is allocating block...\n");

    eDRAMCacheFABlk *victim = tags->findVictim(req.addr);
    assert(victim != nullptr);

    if (victim->isValid())
    {
        evictBlock(victim);
    }

    tags->insertBlock(req.addr, victim, req.new_data.get(), req.old_data.get());
    assert(victim->isValid()); 

    num_of_allos++;
    DPRINTF(Gem5Hacking, "Block %#x is now in eDRAM. "
                         "Current size of eDRAM: %d\n", req.addr,
                         tags->numOccupiedBlocks());
}

void eDRAMCache::evictBlock(eDRAMCacheFABlk *victim)
{
    // Send to write-back queue
    wb_queue->allocate(victim->tag);
    auto ite = wb_queue->data_entries.find(victim->tag);
    assert(ite != wb_queue->data_entries.end());
    eDRAMSimulator::Data_Entry &entry = ite->second;
    victim->retrieve(blkSize, entry.new_data.get(), entry.old_data.get());

    // Invalidate this block
    tags->invalidate(victim);
    assert(!victim->isValid());
    num_of_evics++;

    DPRINTF(Gem5Hacking, "eDRAM is invalidating block %#x. \n", victim->tag);
}

void eDRAMCache::regStats()
{
    MemObject::regStats();

    num_of_allos.name("num_edram_allocs")
                .desc("Number of eDRAM allocations");
    num_of_evics.name("num_edram_evics")
                .desc("Number of eDRAM evictions");
    num_of_write_hits.name("num_of_write_hits")
                     .desc("Number of write hits in eDRAM");
    num_of_read_hits.name("num_of_read_hits")
                    .desc("Number of read hits in eDRAM");
    num_of_wb_hits.name("num_of_wb_hits")
                  .desc("Number of hits in wb queue");
}

eDRAMCache*
eDRAMCacheParams::create()
{
    return new eDRAMCache(this);
}