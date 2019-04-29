#ifndef __PCMSIM_REQUEST_H__
#define __PCMSIM_REQUEST_H__

#include "base/types.hh"
#include "mem/packet.hh"

#include <functional>
#include <memory>
#include <vector>

namespace PCMSim
{
class Request
{
  public:
    Addr addr;

    std::vector<int> addr_vec;

    // Request type
    enum class Request_Type : int
    {
        READ,
        WRITE,
        MAX
    }req_type;

    // clock cycle that request arrives to the queue
    Tick queue_arrival;

    // time to start execution
    Tick begin_exe;

    // estimated completion time
    Tick end_exe;

    // data pointer
    // PCM write is data-aware (e.g. Flip-N-Write) so our PCMSim should be
    // aware of the new data as well as what is already there
    unsigned blkSize;
    std::unique_ptr<uint8_t[]> new_data; // The new value
    std::unique_ptr<uint8_t[]> old_data; // The old value

    // Simulator related
    std::function<void(Request&)> callback;

    Request(Addr _addr, Request_Type _type,
            unsigned _blkSize,
            std::function<void(Request&)> _callback) :
        addr(_addr),
        req_type(_type),
        blkSize(_blkSize),
        new_data(new uint8_t[blkSize]),
        old_data(new uint8_t[blkSize]),
        callback(_callback) {}
};
}
#endif
