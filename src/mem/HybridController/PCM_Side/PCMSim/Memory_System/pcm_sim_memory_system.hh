#ifndef __PCMSIM_MEMORY_SYSTEM_HH__
#define __PCMSIM_MEMORY_SYSTEM_HH__

#include "base/statistics.hh"
#include "debug/Gem5Hacking.hh"
#include "params/PCMSimMemorySystem.hh"

#include "mem/HybridController/PCM_Side/PCMSim/request.hh"
#include "mem/HybridController/PCM_Side/PCMSim/Array_Architecture/pcm_sim_array.hh"
#include "mem/HybridController/PCM_Side/PCMSim/Controller/pcm_sim_controller.hh"
#include "mem/HybridController/PCM_Side/PCMSim/Configs/pcm_sim_config.hh"

#include "sim/sim_object.hh"

#include <math.h>
#include <string>
#include <vector>

namespace PCMSim
{
    class Array;
    class Request;
    class Config;
    class Controller;
}

class PCMSimMemorySystem : public SimObject
{
  public:
    typedef PCMSim::Array Array;
    typedef PCMSim::Request Request;
    typedef PCMSim::Controller Controller;
    typedef PCMSim::Config Config;

  private:
    std::vector<Controller*> controllers;
    std::vector<int> addr_bits;

    const unsigned blkSize; // cache-line size
    const unsigned long long size; // PCM Size
    const unsigned clk_period;

  public:

    PCMSimMemorySystem(PCMSimMemorySystemParams *params);

    ~PCMSimMemorySystem();

    bool send(Request &req);

    void tick();
    
    int numOutstanding();

  private:
    void init(Config &cfgs, float nclks_per_ns);
    
    void decode(Addr _addr, std::vector<int> &vec);

    int sliceLowerBits(Addr& addr, int bits);

  private:
    // I use this function to collect num_diffs and num_sames
    unsigned calcDiff(Request &req)
    {
        unsigned blkSize = req.blkSize;
        uint8_t *new_data = req.new_data.get();
        uint8_t *old_data = req.old_data.get();

	unsigned count = 0;
        for (int i = 0; i < blkSize; i++)
        {
            uint8_t xored = new_data[i]^old_data[i];
            while(xored)
            {
                count += xored & 1;
                xored >>= 1;
            }
        }

        return count;
    }

    void regStats() override;

    // Let's collect information for channel one
    Stats::Scalar rd_queue_size[4];
    Stats::Scalar w_queue_size[4];
    Stats::Formula avg_rd_queue_size[4];
    Stats::Formula avg_w_queue_size[4];

    Stats::Scalar num_accessed;

    // Data information
    Stats::Scalar num_of_reads;
    Stats::Scalar num_of_writes;
    Stats::Scalar num_diffs; // Number of writes whose new data is
                             // different from old data
    Stats::Scalar num_sames; // Number of writes whose new data is
                             // the same as the old data
};

#endif
