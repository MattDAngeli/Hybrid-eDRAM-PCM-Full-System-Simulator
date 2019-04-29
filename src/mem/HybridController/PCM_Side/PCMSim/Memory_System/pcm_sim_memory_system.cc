#include "mem/HybridController/PCM_Side/PCMSim/Memory_System/pcm_sim_memory_system.hh"

PCMSimMemorySystem::PCMSimMemorySystem(PCMSimMemorySystemParams *params)
    : SimObject(params),
      blkSize(params->block_size),
      size(params->size),
      clk_period(params->clock)
{
    // Reading config file
    Config cfgs(params->cfg_file);
    assert(cfgs.blkSize == blkSize);
    assert(cfgs.sizeInGB() == (size / 1024 / 1024 / 1024));
    DPRINTF(Gem5Hacking, "Number of channels: %d\n", cfgs.num_of_channels);
    DPRINTF(Gem5Hacking, "Number of ranks: %d\n", cfgs.num_of_ranks);
    DPRINTF(Gem5Hacking, "Number of banks: %d\n", cfgs.num_of_banks);
    
    float nclks_per_ns = SimClock::Float::ns / clk_period;
    init(cfgs, nclks_per_ns);
}

PCMSimMemorySystem::~PCMSimMemorySystem()
{
    for (auto &c : controllers)
    {
        delete c;
    }
}

void PCMSimMemorySystem::init(Config &cfgs, float nclks_per_ns)
{
    for (int i = 0; i < cfgs.num_of_channels; i++)
    {
        Array *channel = new Array(Config::Level::Channel,
                                   cfgs, nclks_per_ns);
	channel->id = i;

        Controller *controller = new Controller(channel);
        controllers.push_back(controller);
    }

    addr_bits.resize(int(Config::Decoding::MAX));
    addr_bits[int(Config::Decoding::Channel)] =
        int(log2((controllers[0]->channel->arr_info).num_of_channels));
//    DPRINTF(PCMSim, "Decoding bits for channel: %d\n",
//                    addr_bits[int(Config::Decoding::Channel)]);

    addr_bits[int(Config::Decoding::Rank)] =
        int(log2((controllers[0]->channel->arr_info).num_of_ranks));
//    DPRINTF(PCMSim, "Decoding bits for rank: %d\n",
//                    addr_bits[int(Config::Decoding::Rank)]);

    addr_bits[int(Config::Decoding::Row)] =
        int(log2((controllers[0]->channel->arr_info).num_of_word_lines_per_bank));
//    DPRINTF(PCMSim, "Decoding bits for row: %d\n",
//                    addr_bits[int(Config::Decoding::Row)]);

    addr_bits[int(Config::Decoding::Col)] =
        int(log2((controllers[0]->channel->arr_info).num_of_byte_lines_per_bank /
                  blkSize));
//    DPRINTF(PCMSim, "Decoding bits for col: %d\n",
//                    addr_bits[int(Config::Decoding::Col)]);

    addr_bits[int(Config::Decoding::Bank)] =
        int(log2((controllers[0]->channel->arr_info).num_of_banks));
//    DPRINTF(PCMSim, "Decoding bits for bank: %d\n",
//                    addr_bits[int(Config::Decoding::Bank)]);

    addr_bits[int(Config::Decoding::Cache_Line)] = int(log2(blkSize));
//    DPRINTF(PCMSim, "Decoding bits for cache line: %d\n",
//                    addr_bits[int(Config::Decoding::Cache_Line)]);
}

void PCMSimMemorySystem::decode(Addr _addr, std::vector<int> &vec)
{
    Addr addr = _addr;
    for (int i = addr_bits.size() - 1; i >= 0; i--)
    {
        vec[i] = sliceLowerBits(addr, addr_bits[i]);
    }
}

bool PCMSimMemorySystem::send(Request &req)
{
    req.addr_vec.resize(int(Config::Decoding::MAX));
    
    decode(req.addr, req.addr_vec);

    int channel_id = req.addr_vec[int(Config::Decoding::Channel)];

    // Stats - Channel One
    num_accessed++;
    for (int i = 0; i < 4; i++)
    {
        rd_queue_size[i] += controllers[i]->num_of_reads_in_queue;
        w_queue_size[i] += controllers[i]->num_of_writes_in_queue;
    }

    bool is_read = true;
    unsigned diffs = 0;
    if (req.req_type == Request::Request_Type::WRITE)
    {
        diffs = calcDiff(req);
        is_read = false;
    }

    if(controllers[channel_id]->enqueue(req))
    {
        if (is_read)
        {
            num_of_reads++;
        }
        else
        {
            num_of_writes++;
            if (diffs > 0)
            {
                num_diffs++;
            }
            else
            {
                num_sames++;
            }
        }

        return true;
    }

    return false;
}

void PCMSimMemorySystem::tick()
{
    for (auto controller : controllers)
    {
        controller->tick();
    }
}

int PCMSimMemorySystem::numOutstanding()
{
    int outstandings = 0;

    for (auto controller : controllers)
    {
        outstandings += controller->getQueueSize();
    }

    return outstandings;
}

int PCMSimMemorySystem::sliceLowerBits(Addr& addr, int bits)
{
    int lbits = addr & ((1<<bits) - 1);
    addr >>= bits;
    return lbits;
}

void PCMSimMemorySystem::regStats()
{
    SimObject::regStats();

    for (int i = 0; i < 4; i++)
    {
        avg_rd_queue_size[i].name("Channel_" + std::to_string(i) +
                                  "_average_read_queue_size")
                            .desc("average read queue size");
	
        avg_w_queue_size[i].name("Channel_" + std::to_string(i) +
                                  "_average_write_queue_size")
                            .desc("average write queue size");

        avg_rd_queue_size[i] = rd_queue_size[i] / num_accessed;
        avg_w_queue_size[i] = w_queue_size[i] / num_accessed;
    }

    num_of_reads.name("num_of_pcm_reads")
                .desc("Number of PCM reads");

    num_of_writes.name("num_of_pcm_writes")
                .desc("Number of PCM writes");

    num_diffs.name("num_diff_writes")
             .desc("Number of diff writes");

    num_sames.name("num_same_writes")
             .desc("Number of same writes");
}

PCMSimMemorySystem*
PCMSimMemorySystemParams::create()
{
    return new PCMSimMemorySystem(this);
}
