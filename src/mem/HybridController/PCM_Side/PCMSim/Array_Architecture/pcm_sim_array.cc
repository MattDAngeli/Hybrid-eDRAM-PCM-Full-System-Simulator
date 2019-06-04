#include "mem/HybridController/PCM_Side/PCMSim/Array_Architecture/pcm_sim_array.hh"

namespace PCMSim
{

Array::Array(typename Config::Level level_val,
             Config &cfgs, float nclks_per_ns) : level(level_val), id(0)
{
    initArrInfo(cfgs, nclks_per_ns);

    cur_clk = 0;
    next_free = 0;

    int child_level = int(level_val) + 1;
    
    // Stop at bank level
    if (level == Config::Level::Bank)
    {
        return;
    }
    assert(level != Config::Level::Bank);

    int child_max = -1;

    if(level_val == Config::Level::Channel)
    {
        child_max = arr_info.num_of_ranks;
    }
    else if(level_val == Config::Level::Rank)
    {
        child_max = arr_info.num_of_banks;
    }
    assert(child_max != -1);

    for (int i = 0; i < child_max; ++i)
    {
        Array* child = new Array(typename Config::Level(child_level),
                                 cfgs, nclks_per_ns);
        child->parent = this;
        child->id = i;
        children.push_back(child);
    }
}

Array::~Array()
{
    for (auto child: children)
    {
        delete child;
    }
}

void Array::initArrInfo(Config &cfgs, float nclks_per_ns)
{
    arr_info.num_of_word_lines_per_bank = cfgs.num_of_word_lines_per_tile *
                                          cfgs.num_of_parts;

    arr_info.num_of_byte_lines_per_bank = cfgs.num_of_bit_lines_per_tile /
                                          8 *
                                          cfgs.num_of_tiles;
    
    arr_info.num_of_banks = cfgs.num_of_banks; 
    arr_info.num_of_ranks = cfgs.num_of_ranks;
    arr_info.num_of_channels = cfgs.num_of_channels;

    arr_info.tRCD = cfgs.tRCD;
    arr_info.tData = cfgs.tData;
    arr_info.tWL = cfgs.tWL;

    arr_info.nclks_bit_rd = cfgs.ns_bit_rd * nclks_per_ns;
    arr_info.nclks_bit_set = cfgs.ns_bit_set * nclks_per_ns;
    arr_info.nclks_bit_reset = cfgs.ns_bit_reset * nclks_per_ns;

    arr_info.pj_bit_rd = cfgs.pj_bit_rd;
    arr_info.pj_bit_set = cfgs.pj_bit_set;
    arr_info.pj_bit_reset = cfgs.pj_bit_reset;
}

// @ECEC-623, please re-code these two functions.
unsigned Array::write(std::list<Request>::iterator &req)
{
    unsigned lat = arr_info.tRCD + arr_info.tData +
                   arr_info.tWL + arr_info.nclks_bit_set +
                   arr_info.nclks_bit_reset;

    // Latency also must take into account the read-time
    lat += read(req);

    // Copy data from request for manipulation
    unsigned char old_dat[req->blkSize];
    unsigned char new_dat[req->blkSize];
    unsigned char new_dat_f[req->blkSize];

    std::memcpy(old_dat, req->old_data.get(), req->blkSize);
    std::memcpy(new_dat, req->new_data.get(), req->blkSize);

    for (int i=0; i<req->blkSize; i++)
       new_dat_f[i] = ~new_dat[i];

    // Initialize variables for tracking PCM operations
    int sets = 0;
    int resets = 0;
    int sets_f = 0;
    int resets_f = 0;

    int total_bits = 0;

    // Iterate through each element in the data structure
   for (int i=0; i < req->blkSize; i++) {
      // Identify bits to be changed using XOR
      int bits_to_change = old_dat[i] ^ new_dat[i];
      int bits_to_change_f = old_dat[i] ^ new_dat_f[i];
      int bitwidth = sizeof(old_dat[i]) * 8;

      // Shift through the bits of each element to identify
      // if it is being ignored, SET, or RESET
      for (int j=0; j < bitwidth; j++) {

         // Normal data
         // Change the current bit? (right-most)
         int change_bit = bits_to_change & 1;

         if ( change_bit == 1 ) {

            // What is the new bit going to be? 1 or 0?
            int new_bit = new_dat[i] & 1;

            if ( new_bit == 1 ) {
               sets += 1;
            } else {
               resets += 1;
            }
         }
         new_dat[i] >>= 1;
         bits_to_change >>= 1;

         // Flipped data
         int change_bit_f = bits_to_change_f & 1;

         if ( change_bit_f == 1 ) {

            int new_bit_f = new_dat_f[i] & 1;

            if ( new_bit_f == 1 ) {
               sets_f += 1;
            } else {
               resets_f += 1;
            }
         }
         new_dat_f[i] >>= 1;
         bits_to_change_f >>= 1;

         total_bits++;
      }
   }

   // Identify total number of operations
   int operations = sets + resets;

   // Output stats
   std::cout << "Normal:" << std::endl <<
      "\tSets: " << sets <<
      "\tResets: " << resets <<
      "\tTotal Bits: " << total_bits << std:: endl;
   std::cout << "Flipped:" << std::endl <<
     "\tSets: " << sets_f <<
     "\tResets: " << resets_f <<
     "\tTotal Bits: " << total_bits << std::endl;
   std::cout << std::endl << (operations < total_bits/2 ? "Not " : "") <<
      "Flipping Data" << std::endl;
   std::cout << "=========================================" << std::endl;

    return lat;
}

unsigned Array::read(std::list<Request>::iterator &req)
{
    unsigned lat = arr_info.tRCD + arr_info.tData +
                   arr_info.nclks_bit_rd;
    return lat;
}

}
