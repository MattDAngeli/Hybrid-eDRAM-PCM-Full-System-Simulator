# import the parser
from optparse import OptionParser
parser = OptionParser()
parser.add_option('--workload_dir', help="workload directory")
parser.add_option('--eDRAM_cache_size', help="eDRAM cache size")
parser.add_option("-w", action="store_true",\
                  dest="eDRAM_cache_write_only_mode",
                  help="eDRAM cache will cache writes only")
parser.add_option("-n", action="store_false",\
                  dest="eDRAM_cache_write_only_mode",
                  help="eDRAM cache will cache both reads and writes (normal)")
parser.add_option('--eDRAM_cache_read_partition', help="How much eDRAM for reads")
parser.add_option('--eDRAM_cache_write_partition', help="How much for writes")
(options, args) = parser.parse_args()

from os.path import join

# import the m5 (gem5) library created when gem5 is built
import m5

# import all of the SimObjects
from m5.objects import *

#m5.util.addToPath("simulation")
from caches import *

# create the system we are going to simulate
system = System()
system.mmap_using_noreserve = True # When simulating large memory

# Set the clock fequency of the system (and all of its children)
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '3.32GHz'
system.clk_domain.voltage_domain = VoltageDomain()

# Set up the system
system.mem_mode = 'timing'               # Use timing accesses
system.mem_ranges = [AddrRange('128GB')] # Create an address range

# Create a simple CPU
system.cpu = TimingSimpleCPU()
system.cpu.max_insts_all_threads = 1000000

# Create a memory bus, eDRAM and PCM
system.membus = SystemXBar() # eDRAM to PCM
system.hybrid = HybridController() # PCM (Main Memory)
system.hybrid.eDRAM_cache_size = options.eDRAM_cache_size
system.hybrid.eDRAM_cache_write_only_mode = options.eDRAM_cache_write_only_mode
system.hybrid.eDRAM_cache_read_partition = options.eDRAM_cache_read_partition
system.hybrid.eDRAM_cache_write_partition = options.eDRAM_cache_write_partition

# Hook up the CPU ports
system.cpu.icache = L1ICache()
system.cpu.dcache = L1DCache()

system.cpu.icache_port = system.cpu.icache.cpu_side
system.cpu.dcache_port = system.cpu.dcache.cpu_side

system.cpu.icache.mem_side = system.membus.slave
system.cpu.dcache.mem_side = system.membus.slave

# create the interrupt controller for the CPU and connect to the membus
system.cpu.createInterruptController()

# Hook up eDRAM-PCM
system.hybrid.range = system.mem_ranges[0]
system.hybrid.cpu_side_port = system.membus.master

# Connect the system up to the membus
system.system_port = system.membus.slave

# Setup a testing application
process = Process(pid = 100)
prog_dir = join(options.workload_dir, "505.mcf_r/")
process.cmd = [prog_dir + "mcf_r", prog_dir + "inp.in"]

# Set the cpu to use the process as its workload and create thread contexts
system.cpu.workload = process
system.cpu.createThreads()

# set up the root SimObject and start the simulation
root = Root(full_system = False, system = system)
# instantiate all of the objects we've created above
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
