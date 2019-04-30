// For hacking Gem5
#include "mem/HybridController/hybrid_controller.hh"
#include "mem/HybridController/eDRAM_Cache_Side/eDRAM_cache.hh"
#include "mem/HybridController/PCM_Side/PCM.hh"

HybridController::HybridController(const HybridControllerParams *params) :
    AbstractMemory(params),
    cpu_side_port(params->name + ".cpu_side_port", this),
    send_resp_event(this),
    tick_event(this),
    resp_stall(false),
    req_stall(false),
    eDRAM(params->eDRAM),
    pcm(params->main_memory)
{
    ticks_per_clk = clockPeriod();
    DPRINTF(Gem5Hacking, "Hybrid controller clock period: %lld\n",
                         static_cast<unsigned long long>(ticks_per_clk));
}

int HybridController::numOutstanding()
{
    return pcm->numOutstanding() + resp_queue.size();
}

void HybridController::init()
{
    AbstractMemory::init();

    if (!cpu_side_port.isConnected())
    {
        fatal("HybridController %s is unconnected!\n", name());
    }
    else
    {
        cpu_side_port.sendRangeChange();
    }

    eDRAM->setPCM(pcm); // Give eDRAM interface to PCM for sending
                        // write-back requests
    eDRAM->setHybridC(this); // Give eDRAM access to hybrid controller

    pcm->setHybridC(this); // Give PCM access to hybrid controller
}

void HybridController::processTickEvent()
{
    if (req_stall)
    {
        req_stall = false;
        cpu_side_port.sendRetryReq();
    }

    schedule(tick_event, curTick() + ticks_per_clk);
}

void HybridController::startup()
{
    schedule(tick_event, clockEdge());
}

DrainState HybridController::drain()
{
    if (numOutstanding())
    {
        return DrainState::Draining;
    }
    else
    {
        return DrainState::Drained;
    }
}

BaseSlavePort&
HybridController::getSlavePort(const std::string& if_name, PortID idx)
{
    if (if_name == "cpu_side_port")
    {
        return cpu_side_port;
    }

    fatal("Error in getSlavePort()");
}

AddrRangeList HybridController::CPUSidePort::getAddrRanges() const
{
    AddrRangeList ranges;
    ranges.push_back(hybridC->getAddrRange());
    return ranges;
}

Tick HybridController::CPUSidePort::recvAtomic(PacketPtr pkt)
{
    return hybridC->recvAtomic(pkt);
}

void HybridController::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    hybridC->recvFunctional(pkt);
}

bool HybridController::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    DPRINTF(Gem5Hacking, "PCM received request for addr %#x\n",
                         pkt->getAddr());

    return hybridC->recvTimingReq(pkt);
}

void HybridController::CPUSidePort::recvRespRetry()
{
    hybridC->recvRespRetry();
}

void HybridController::sendResponse()
{
    assert(!resp_stall);
    assert(!resp_queue.empty());

    if (cpu_side_port.sendTimingResp(resp_queue.front()))
    {
        DPRINTF(Gem5Hacking, "PCM is sending response for addr %#x\n",
                             resp_queue.front()->getAddr());

        resp_queue.pop_front();
        if (resp_queue.size() && !send_resp_event.scheduled())
        {
            schedule(send_resp_event, curTick());
        }

        if (numOutstanding() == 0)
        {
            signalDrainDone();
        }
    }
    else
    {
        resp_stall = true;
    }
}

Tick HybridController::recvAtomic(PacketPtr pkt)
{
    access(pkt);

    Tick latency = 0;

    return latency;
}

void HybridController::recvFunctional(PacketPtr pkt)
{
    functionalAccess(pkt);
}

bool HybridController::recvTimingReq(PacketPtr pkt)
{
    DPRINTF(PCMSim, "receiving...\n");
    panic_if(pkt->cacheResponding(), "Should not see packets where cache "
             "is responding");

    panic_if(!(pkt->isRead() || pkt->isWrite()),
             "Should only see read and writes at memory controller\n");

    assert(!req_stall);

    /* eDRAM access */
    // The current eDRAM is configured to serve as a cache for
    // PCM writes. However, it's very easy to configure it for
    // PCM reads as well.
    bool hit_in_eDRAM = eDRAM->access(pkt);
    if (hit_in_eDRAM == true)
    {
        return true;
    }
    else
    {
        if ((pkt->isWrite() && eDRAM->write_only) || !eDRAM->write_only)
        {
            // In write only mode, we don't allow write request to be directly
            // sent to PCM
            // In normal caching mode, we don't allow any requests to be directly
            // sent to PCM
            req_stall = true;
            return false;
        }
    }

    // At this point, eDRAM must be in write_only mode and there must be a read
    // packet
    assert(eDRAM->write_only);
    assert(pkt->isRead());

    /* PCM access */
    bool accepted = pcm->access(pkt);
    if (!accepted)
    {
        req_stall = true;
        return false;
    }

    return true;
}

void HybridController::recvRespRetry()
{
    assert(resp_stall); // responding must already be stalled
    resp_stall = false;
    sendResponse();
}

void HybridController::accessAndRespond(PacketPtr pkt)
{
    bool needsResponse = pkt->needsResponse();

    // Simply write/read data to/from physical memory
    access(pkt);

    if (needsResponse)
    {
        Tick time = curTick() + pkt->headerDelay + pkt->payloadDelay +
                    eDRAM->getTagDelay(); // need to count tag lookup latency

        // Here we reset the timing of the packet before sending it out.
        pkt->headerDelay = pkt->payloadDelay = 0;

	resp_queue.push_back(pkt);

        if (!resp_stall && !send_resp_event.scheduled())
        {
            schedule(send_resp_event, time);
        }
    }
    else
    {
        pendingDelete.reset(pkt);
    }
}

void HybridController::dataRetrieval(PacketPtr pkt, Request &req)
{
    dataRetrieval(pkt, req.new_data.get(), req.old_data.get());
}

void HybridController::dataRetrieval(PacketPtr pkt,
                                     uint8_t *new_data,
                                     uint8_t *old_data)
{
    assert(new_data != nullptr);
    assert(old_data != nullptr);

    // PCM write is usually data aware which means the write driver 
    // needs to be aware of new data as well as the old data
    unsigned blkSize = pkt->getSize();

    // Step one, retrieve old data from physical memory
    assert(AddrRange(pkt->getAddr(),
                     pkt->getAddr() + (pkt->getSize() - 1)).isSubset(range));
    uint8_t *hostAddr = pmemAddr + pkt->getAddr() - range.start();
    std::memcpy(old_data, hostAddr, blkSize);

    // Step two, retrieve new data
    if (pkt->isWrite())
    {
        std::memcpy(new_data, pkt->getConstPtr<uint8_t>(), blkSize);
    }
    else
    {
        // For reads, new data is the same as old data.
        std::memcpy(new_data, hostAddr, blkSize);
    }
}

HybridController *
HybridControllerParams::create()
{
    return new HybridController(this);
}
