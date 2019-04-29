import m5
from m5.objects import CoherentXBar,SnoopFilter

# We assume eDRAM is on chip
class eDRAMXBar(CoherentXBar):
    width = 32

    frontend_latency = 1
    forward_latency = 0
    response_latency = 1
    snoop_response_latency = 1

    snoop_filter = SnoopFilter(lookup_latency = 1)

    # There are no coherent downstream caches
    # This should be important
    point_of_coherency = True

    point_of_unification = True
