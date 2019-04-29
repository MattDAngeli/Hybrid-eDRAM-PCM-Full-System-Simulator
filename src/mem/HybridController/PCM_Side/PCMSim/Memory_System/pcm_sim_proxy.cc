#include "mem/HybridController/PCM_Side/PCMSim/Memory_System/pcm_sim_proxy.hh"

PCMSimProxy*
PCMSimProxyParams::create()
{
    return new PCMSimProxy(this);
}
