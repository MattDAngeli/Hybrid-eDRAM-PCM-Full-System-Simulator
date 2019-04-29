#include "mem/HybridController/PCM_Side/PCM.hh"

#include "mem/HybridController/hybrid_controller.hh"

PCM::PCM(const PCMParams *params)
    : MemObject(params),
      proxy(params->pcm_proxy),
      tick_event(this),
      read_cb_func(std::bind(&PCM::readComplete, this,
                             std::placeholders::_1)),
      write_cb_func(std::bind(&PCM::writeComplete, this,
                             std::placeholders::_1)),
      ticks_per_clk(params->clock),
      blkSize(params->block_size),
      size(params->size)
{
    DPRINTF(Gem5Hacking, "PCM is initiated. \n");
    DPRINTF(Gem5Hacking, "PCM size (GB): %u\n", size / 1024 / 1024 / 1024);
    DPRINTF(Gem5Hacking, "PCM clock period: %lld\n",
                         static_cast<unsigned long long>(ticks_per_clk));
}

int PCM::numOutstanding()
{
    return proxy->numOutstanding();
}

void PCM::startup()
{
    schedule(tick_event, clockEdge());
}

void PCM::processTickEvent()
{
    proxy->tick();
    schedule(tick_event, curTick() + ticks_per_clk);
}

bool PCM::access(Request &req)
{
    return proxy->send(req);
}

bool PCM::access(PacketPtr pkt)
{
    if (pkt->isRead())
    {
        return processReadReq(pkt);
    }
    else if (pkt->isWrite())
    {
        return processWriteReq(pkt);
    }

    fatal("Should never happen");
    return false;
}

bool PCM::processReadReq(PacketPtr pkt)
{
    assert(!pkt->isWrite());

    Addr addr = pkt->getAddr();

    if (isInWriteQueue.find(addr) != isInWriteQueue.end())
    {
        // Packet can be merged by write queue, simply respond back
        hybridC->accessAndRespond(pkt);
        return true;
    }

    // Not found in the write queue, push it into the read queue.
    Request req(pkt->getAddr(),
                Request::Request_Type::READ,
                blkSize,
                read_cb_func);
    bool accepted = proxy->send(req);
    
    if (accepted)
    {
        outstandingReads[addr].push_back(pkt);
        return true;
    }

    return false; // Our PCMSim is busy
}

bool PCM::processWriteReq(PacketPtr pkt)
{
    assert(!pkt->isRead());

    Addr addr = pkt->getAddr();

    // For future consistency check
    isInWriteQueue.insert(addr);

    Request req(pkt->getAddr(),
                Request::Request_Type::WRITE,
                blkSize,
                write_cb_func);
    // For any write request to PCMSim, it needs have
    // both new data and old data.
    hybridC->dataRetrieval(pkt, req);
    bool accepted = proxy->send(req);

    if (accepted)
    {
        outstandingWrites[addr].push_back(pkt);
        hybridC->accessAndRespond(pkt);

	return true;
    }

    return false; // Our PCMSim is busy
}

void PCM::readComplete(Request& req)
{
    auto p = outstandingReads.find(req.addr);
    assert(p != outstandingReads.end());

    PacketPtr pkt = p->second.front();
    p->second.pop_front();

    if (p->second.empty())
    {
        outstandingReads.erase(p);
    }

    hybridC->accessAndRespond(pkt);
}

void PCM::writeComplete(Request& req)
{
    auto p = outstandingWrites.find(req.addr);
    assert(p != outstandingWrites.end());

    p->second.pop_front();
    isInWriteQueue.erase(req.addr); // Retire the write address

    if (p->second.empty())
    {
        outstandingWrites.erase(p);
    }
    
    if (numOutstanding())
    {
        hybridC->signalDrainDone();
    }
}

PCM*
PCMParams::create()
{
    return new PCM(this);
}

