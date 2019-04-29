#ifndef __DEFERRED_QUEUE_HH__
#define __DEFERRED_QUEUE_HH__

#include <assert.h>
#include <memory>
#include <set>
#include <unordered_map>

namespace eDRAMSimulator
{
struct Data_Entry
{
    Data_Entry(unsigned _blkSize)
        : blkSize(_blkSize),
          new_data(new uint8_t[blkSize]),
          old_data(new uint8_t[blkSize])
    {}
    unsigned blkSize;

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
    virtual void allocate(Addr addr)
    {
        data_entries.insert({addr, Data_Entry(blkSize)});
    }

    virtual void deAllocate(Addr addr, bool on_board = true)
    {
        assert(data_entries.erase(addr));
    }

    virtual bool isInQueue(Addr addr)
    {
        bool in_all = (all_entries.find(addr) != all_entries.end());
        bool not_on_board = (entries_on_flight.find(addr) ==
                             entries_on_flight.end());
        return (in_all && not_on_board);
    }

    typedef std::unordered_map<Addr, Data_Entry> DataEntryHash;
    DataEntryHash data_entries;

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

    void allocate(Addr addr) override
    {
        auto ret = all_entries.insert(addr);

        // To prevent the same addr is inserted again.
        if (ret.second == true)
	{
            Deferred_Queue::allocate(addr);
	}

        // Make sure the entry is there
        assert(data_entries.find(addr) != data_entries.end());
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
        assert(data_entries.find(addr) == data_entries.end());
    }
};
}

#endif
