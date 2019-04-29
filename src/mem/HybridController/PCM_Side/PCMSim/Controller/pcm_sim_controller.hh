#ifndef __PCMSIM_CONTROLLER_HH__
#define __PCMSIM_CONTROLLER_HH__

#include "base/trace.hh"
#include "debug/PCMSim.hh"

#include "mem/HybridController/PCM_Side/PCMSim/request.hh"
#include "mem/HybridController/PCM_Side/PCMSim/Array_Architecture/pcm_sim_array.hh"

#include <algorithm>
#include <list>
#include <iostream>
#include <iomanip>
#include <deque>

namespace PCMSim
{
class Controller
{
  public:
    Array *channel;

  private:
    Tick clk;

    std::list<Request> r_w_q;
    const int max = 64; // Max size of r_w_q

    std::deque<Request> r_w_pending_queue;

  public:
    unsigned num_of_reads_in_queue;
    unsigned num_of_writes_in_queue;

  private:
    // This section contains scheduler
    bool scheduled;
    std::list<Request>::iterator scheduled_req;

  public:
    Controller(Array *_channel) : channel(_channel), clk(0),
                                  num_of_reads_in_queue(0),
                                  num_of_writes_in_queue(0),
                                  scheduled(0) {}
    ~Controller() { delete channel; }

    int getQueueSize() { return r_w_q.size(); }
    
    bool enqueue(Request& req)
    {
        if (r_w_q.size() == max)
        {
            // Queue is full
            return false;
        }
        
	r_w_q.push_back(std::move(req));
        if (req.req_type == Request::Request_Type::READ)
        {
            num_of_reads_in_queue++;
        }
        else
        {
            num_of_writes_in_queue++;
        }

        return true;
    }

    void tick();

  private:
    bool issueAccess();
    void servePendingAccesses();
    void channelAccess();
};
}

#endif
