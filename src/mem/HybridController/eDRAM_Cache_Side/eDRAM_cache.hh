#ifndef __eDRAMCACHE_HH__
#define __eDRAMCACHE_HH__

#include "params/eDRAMCache.hh"
#include "debug/Gem5Hacking.hh"

#include "mem/mem_object.hh"
#include "mem/HybridController/eDRAM_Cache_Side/tags/eDRAM_cache_tags.hh"
#include "mem/HybridController/eDRAM_Cache_Side/util/deferred_queue.hh"
#include "mem/HybridController/PCM_Side/PCMSim/request.hh"

#include <unordered_map>

namespace eDRAMSimulator
{
    class Deferred_Set;
}
namespace PCMSim
{
    class Request;
}

class HybridController;
class PCM;

class eDRAMCache : public MemObject
{
  public:
    typedef PCMSim::Request Request;

    eDRAMCache(const eDRAMCacheParams *params);

    ~eDRAMCache()
    {
        delete mshrs;
        delete wb_queue;
    }
    virtual void startup();

    bool blocked() { return (mshrs->isFull() || wb_queue->isFull()); }

    void setPCM(PCM *_pcm) { pcm = _pcm; }
    void setHybridC(HybridController *_hybridC) { hybridC = _hybridC; }

    bool access(PacketPtr pkt);

    Tick getTagDelay() { return tag_delay; }

    bool write_only; // Only cache writes or both reads and writes

  private:
    // TODO, we need to drain the MSHRS
    std::unordered_map<Addr, std::deque<PacketPtr>> outstandingReads;

  private:
    // We rely on tick() to send either an MSHR request or a write-back request
    void processTickEvent();
    EventWrapper<eDRAMCache, &eDRAMCache::processTickEvent> tick_event;

  private:
    // To send either an MSHR or WB request
    void sendDeferredReq();

  // This section deals with MSHR handling and WB handling
  private:
    std::function<void(Request&)> mshr_cb_func;
    std::function<void(Request&)> wb_cb_func;

    void sendMSHRReq(Addr addr);
    void MSHRComplete(Request& req);

    void sendWBReq(Addr addr);
    void WBComplete(Request& req);

    void allocateBlock(Request& req);
    void evictBlock(eDRAMCacheFABlk *victim);

  private:
    HybridController *hybridC;
    PCM *pcm; // Interface to PCM

    unsigned blkSize;
    Tick ticks_per_clk;
    Tick tag_delay;

    eDRAMCacheTagsWithFABlk *tags;
    eDRAMSimulator::Deferred_Set *mshrs;
    eDRAMSimulator::Deferred_Set *wb_queue;

  private:
    void regStats() override;

    Stats::Scalar num_of_read_allos;
    Stats::Scalar num_of_write_allos;
    Stats::Scalar num_of_evics;
    Stats::Scalar num_of_write_hits;
    Stats::Scalar num_of_read_hits;
    Stats::Scalar num_of_wb_hits;
};

#endif
