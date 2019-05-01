#ifndef __DEFERRED_QUEUE_HH__
#define __DEFERRED_QUEUE_HH__

#include <assert.h>
#include <memory>
#include <set>
#include <unordered_map>

#include "mem/HybridController/PCM_Side/PCMSim/request.hh"

namespace PCMSim
{
    class Request;
}

namespace eDRAMSimulator
{
struct Entry
{
    Entry(unsigned _blkSize, PCMSim::Request::MSHR_Type t_mshr_type)
        : blkSize(_blkSize),
          mshr_type(t_mshr_type),
          new_data(new uint8_t[blkSize]),
          old_data(new uint8_t[blkSize])
    {}
    
    unsigned blkSize;

    PCMSim::Request::MSHR_Type mshr_type;

    void retrieve(uint8_t *_new_data, uint8_t *_old_data)
    {
        assert(_new_data != nullptr);
        assert(_old_data != nullptr);
        std::memcpy(_new_data, new_data.get(), blkSize);
        std::memcpy(_old_data, old_data.get(), blkSize);
    }

    std::unique_ptr<uint8_t[]> new_data;
    std::unique_ptr<uint8_t[]> old_data;
};

template<class T>
class Deferred_Queue
{
  public:
    Deferred_Queue(int _max, unsigned _blkSize) : max(_max), blkSize(_blkSize) {}
    virtual ~Deferred_Queue() {}

    bool isFull() { return all_entries.size() == max; }
    int numEntries() { return all_entries.size(); }
    
    bool getEntry(Addr &entry)
    {
        // This entry should not be on flight
        for (auto ite = all_entries.begin(); ite != all_entries.end(); ite++)
        {
            if (entries_on_flight.find(*ite) == entries_on_flight.end())
            {
                // Good, we haven't sent out this entry
                entry = *ite;
                return true;
            }
        }
        
        return false;
    }

    virtual void entryOnBoard(Addr addr) {}
    virtual void allocate(Addr addr,
                          PCMSim::Request::MSHR_Type t_mshr_type)
    {
        entries.insert({addr, Entry(blkSize, t_mshr_type)});
    }

    virtual void deAllocate(Addr addr, bool on_board = true)
    {
        assert(entries.erase(addr));
    }

    virtual bool isInQueue(Addr addr)
    {
        bool in_all = (all_entries.find(addr) != all_entries.end());
        bool not_on_board = (entries_on_flight.find(addr) ==
                             entries_on_flight.end());
        return (in_all && not_on_board);
    }

    typedef std::unordered_map<Addr, Entry> EntryHash;
    EntryHash entries;

  protected:
    int max; // max number of MSHR entries
    T all_entries;
    T entries_on_flight;

    unsigned blkSize;
};

class Deferred_Set : public Deferred_Queue<std::set<Addr>>
{
public:
    Deferred_Set(int max, unsigned blkSize)
        : Deferred_Queue<std::set<Addr>>(max, blkSize) {}

    void entryOnBoard(Addr addr) override
    {
        entries_on_flight.insert(addr);
    }

    void allocate(Addr addr,
                  PCMSim::Request::MSHR_Type t_mshr_type) override
    {
        auto ret = all_entries.insert(addr);

        // To prevent the same addr is inserted again.
        if (ret.second == true)
	{
            Deferred_Queue::allocate(addr, t_mshr_type);
	}
        else
        {
            auto ite = entries.find(addr);
            assert(ite != entries.end());
            Entry &entry = ite->second;
            // I double it will ever happen
            if (entry.mshr_type != PCMSim::Request::MSHR_Type::MAX &&
                entry.mshr_type != t_mshr_type)
            {
                entry.mshr_type = PCMSim::Request::MSHR_Type::LS;
            }
        }

        // Make sure the entry is there
        assert(entries.find(addr) != entries.end());
    }

    void deAllocate(Addr addr, bool on_board = true) override
    {
        assert(all_entries.erase(addr));
        if (on_board)
        {
            assert(entries_on_flight.erase(addr));
        }

        Deferred_Queue::deAllocate(addr);

        // Double make sure the entry is gone.
        assert(entries.find(addr) == entries.end());
    }
};
}

#endif
