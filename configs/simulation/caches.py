import m5
from m5.objects import Cache

class L1Cache(Cache):
    assoc = 8
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    mshrs = 4
    tgts_per_mshr = 20

class L1ICache(L1Cache):
    size = '32kB'
    is_read_only = True
    writeback_clean = False

class L1DCache(L1Cache):
    size = '32kB'
    is_read_only = False
    writeback_clean = False

class L2Cache(Cache):
    size = '512kB'

    assoc = 8
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12
    write_buffers = 8

class L3Cache(Cache):
    size = '8MB'

    assoc = 16
    tag_latency = 20
    data_latency = 20
    sequential_access = True
    response_latency = 40
    mshrs = 32
    tgts_per_mshr = 12
    write_buffers = 16
