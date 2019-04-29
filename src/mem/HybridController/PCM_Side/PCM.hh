#ifndef __PCM_HH__
#define __PCM_HH__

#include "params/PCM.hh"
#include "debug/Gem5Hacking.hh"

#include "mem/mem_object.hh"

#include "mem/HybridController/PCM_Side/PCMSim/Memory_System/pcm_sim_proxy.hh"
#include "mem/HybridController/PCM_Side/PCMSim/request.hh"

#include <unordered_map>

namespace PCMSim
{
    class Request;
}

class HybridController;

class PCM : public MemObject
{
  public:
    typedef PCMSim::Request Request;

    PCM(const PCMParams *params);

    virtual void startup();

    bool access(PacketPtr pkt); // Interface to hybrid memory controller
    bool access(Request &req); // Interface to eDRAM

    // Set the hybrid controller that controlls the PCM
    void setHybridC(HybridController *_hybridC) { hybridC = _hybridC; }

    // Number of requests inside PCM controller's queue
    int numOutstanding();

  private:
    HybridController *hybridC; // The hybrid controller which controlls the PCM
    PCMSimProxy *proxy; // Proxy to our stand-alone PCM simulator

  private:
    void processTickEvent();
    EventWrapper<PCM, &PCM::processTickEvent> tick_event;

  private:
    std::function<void(Request&)> read_cb_func;
    std::function<void(Request&)> write_cb_func;

  private:
    // I think unordered_map<Addr, PacketPtr> should also work
    std::unordered_map<Addr, std::deque<PacketPtr>> outstandingReads;
    std::unordered_map<Addr, std::deque<PacketPtr>> outstandingWrites;

    std::set<Addr> isInWriteQueue;

  private:
    bool processReadReq(PacketPtr pkt);
    bool processWriteReq(PacketPtr pkt);
    void readComplete(Request& req);
    void writeComplete(Request& req);

  private:
    Tick ticks_per_clk;

    const unsigned blkSize; // cache-line (block) size
    const unsigned long long size; // PCM Size
};

#endif
