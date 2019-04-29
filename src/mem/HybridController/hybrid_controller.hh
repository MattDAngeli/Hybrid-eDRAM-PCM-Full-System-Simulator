#ifndef __HYBRID_CONTROLLER_HH__
#define __HYBRID_CONTROLLER_HH__

#include "debug/Gem5Hacking.hh"
#include "mem/abstract_mem.hh"
#include "mem/HybridController/PCM_Side/PCMSim/request.hh"
#include "params/HybridController.hh"

class eDRAMCache;
class PCM;

namespace PCMSim
{
    class Request;
}

class HybridController : public AbstractMemory
{
    typedef PCMSim::Request Request;

  private:
    class CPUSidePort : public SlavePort
    {
      private:
        HybridController *hybridC;

      public:
        CPUSidePort(const std::string &_name, HybridController *_hybridC) :
            SlavePort(_name, _hybridC),
            hybridC(_hybridC)
        { }

        AddrRangeList getAddrRanges() const override;

      protected:
        Tick recvAtomic(PacketPtr pkt) override;
        void recvFunctional(PacketPtr pkt) override;
        bool recvTimingReq(PacketPtr pkt) override;
        void recvRespRetry() override;
    };
    CPUSidePort cpu_side_port;

  private:
    Tick ticks_per_clk;

  private:
    void sendResponse();
    EventWrapper<HybridController, &HybridController::sendResponse> send_resp_event;

    void processTickEvent();
    EventWrapper<HybridController, &HybridController::processTickEvent> tick_event;

  private:
    bool resp_stall;
    bool req_stall;

    std::deque<PacketPtr> resp_queue;
    
    int numOutstanding();

  public:
    HybridController(const HybridControllerParams *params);

    BaseSlavePort& getSlavePort(const std::string& if_name, PortID idx);

    virtual void init();
    virtual void startup();
    DrainState drain() override;

  private:
    Tick recvAtomic(PacketPtr pkt);
    void recvFunctional(PacketPtr pkt);
    bool recvTimingReq(PacketPtr pkt);
    void recvRespRetry();

  private:
    void accessAndRespond(PacketPtr pkt);
    void dataRetrieval(PacketPtr pkt, Request &req);
    void dataRetrieval(PacketPtr pkt,
                       uint8_t *new_data,
                       uint8_t *old_data);

  private:
    // I'm not sure why this is needed. From comments of DRAMSIM2, it seems
    // that upstream caches need this packet until true is returned, so
    // hold it for deletion until a subsequent call.
    std::unique_ptr<Packet> pendingDelete;

  private:
    eDRAMCache *eDRAM;
    PCM *pcm;

  friend class eDRAMCache;
  friend class PCM;
};

#endif
