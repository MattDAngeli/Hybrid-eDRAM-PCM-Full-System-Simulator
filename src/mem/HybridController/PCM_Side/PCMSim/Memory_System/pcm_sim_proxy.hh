#ifndef __PCMSIM_PROXY_HH__
#define __PCMSIM_PROXY_HH__

#include "debug/Gem5Hacking.hh"
#include "params/PCMSimProxy.hh"

#include "mem/HybridController/PCM_Side/PCMSim/Memory_System/pcm_sim_memory_system.hh"
#include "mem/HybridController/PCM_Side/PCMSim/request.hh"

#include "sim/sim_object.hh"


namespace PCMSim
{
    class Request;
}

class PCMSimProxy : public SimObject
{
  private:
    PCMSimMemorySystem *PCMSim_system; // Contains all the controllers

  public:
    PCMSimProxy(PCMSimProxyParams *params)
        : SimObject(params),
          PCMSim_system(params->mem_system)
    {
        DPRINTF(Gem5Hacking, "PCMSim Proxy is setup. \n");
    }
   
    typedef PCMSim::Request Request;

    int numOutstanding()
    {
        return PCMSim_system->numOutstanding();
    }

    void tick()
    {
        PCMSim_system->tick();
    }

    bool send(Request &req)
    {
        return PCMSim_system->send(req);
    }
};

#endif
