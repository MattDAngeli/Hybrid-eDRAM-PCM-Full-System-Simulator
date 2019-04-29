# import the parser
from optparse import OptionParser
parser = OptionParser()
parser.add_option('--workload_dir', help="workload directory")
(options, args) = parser.parse_args()

from os.path import join

# import the m5 (gem5) library created when gem5 is built
import m5
# import all of the SimObjects
from m5.objects import *

# import custom caches
m5.util.addToPath("../../")
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

###################### Define Workloads ###################### 
np = 12
MAX_INSTS = 100000000

multiprocesses = []


process = Process(pid = 100)
prog_dir = join(options.workload_dir, "502.gcc_r/")
process.cmd = [prog_dir + "cpugcc_r", prog_dir + "ref32.c", "-O3", "-fselective-scheduling", "-fselective-scheduling2", "-o", prog_dir + "out"]
multiprocesses.append(process)

process = Process(pid = 101)
prog_dir = join(options.workload_dir, "502.gcc_r/")
process.cmd = [prog_dir + "cpugcc_r", prog_dir + "ref32.c", "-O3", "-fselective-scheduling", "-fselective-scheduling2", "-o", prog_dir + "out"]
multiprocesses.append(process)

process = Process(pid = 102)
prog_dir = join(options.workload_dir, "523.xalancbmk_r/")
process.cmd = [prog_dir + "cpuxalan_r", "-v", prog_dir + "t5.xml", prog_dir + "xalanc.xsl"]
multiprocesses.append(process)

process = Process(pid = 103)
prog_dir = join(options.workload_dir, "523.xalancbmk_r/")
process.cmd = [prog_dir + "cpuxalan_r", "-v", prog_dir + "t5.xml", prog_dir + "xalanc.xsl"]
multiprocesses.append(process)

process = Process(pid = 104)
prog_dir = join(options.workload_dir, "544.nab_r/")
process.cmd = [prog_dir + "nab_r", "1am0", "1122214447", "122"]
multiprocesses.append(process)

process = Process(pid = 105)
prog_dir = join(options.workload_dir, "544.nab_r/")
process.cmd = [prog_dir + "nab_r", "1am0", "1122214447", "122"]
multiprocesses.append(process)

process = Process(pid = 106)
prog_dir = join(options.workload_dir, "519.lbm_r/")
process.cmd = [prog_dir + "lbm_r", "3000", "reference.dat", "0", "0", prog_dir + "100_100_130_ldc.of"]
multiprocesses.append(process)

process = Process(pid = 107)
prog_dir = join(options.workload_dir, "519.lbm_r/")
process.cmd = [prog_dir + "lbm_r", "3000", "reference.dat", "0", "0", prog_dir + "100_100_130_ldc.of"]
multiprocesses.append(process)

process = Process(pid = 108)
prog_dir = join(options.workload_dir, "538.imagick_r/")
process.cmd = [prog_dir + "imagick_r", "-limit", "disk", "0", prog_dir + "refrate_input.tga", "-edge", "41", "-resample", "181%", "-emboss", "31", "-colorspace", "YUV", "-mean-shift", "19x19+15%", "-resize", "30%", prog_dir + "refrate_output.tga"]
multiprocesses.append(process)

process = Process(pid = 109)
prog_dir = join(options.workload_dir, "538.imagick_r/")
process.cmd = [prog_dir + "imagick_r", "-limit", "disk", "0", prog_dir + "refrate_input.tga", "-edge", "41", "-resample", "181%", "-emboss", "31", "-colorspace", "YUV", "-mean-shift", "19x19+15%", "-resize", "30%", prog_dir + "refrate_output.tga"]
multiprocesses.append(process)

process = Process(pid = 110)
prog_dir = join(options.workload_dir, "531.deepsjeng_r/")
process.cmd = [prog_dir + "deepsjeng_r", prog_dir + "ref.txt"]
multiprocesses.append(process)

process = Process(pid = 111)
prog_dir = join(options.workload_dir, "531.deepsjeng_r/")
process.cmd = [prog_dir + "deepsjeng_r", prog_dir + "ref.txt"]
multiprocesses.append(process)


###################### Define CPUs ###################### 
system.membus = SystemXBar()

system.l3bus = L2XBar()
system.l3cache = L3Cache()
system.l3cache.cpu_side = system.l3bus.master
system.l3cache.mem_side = system.membus.slave # LLC to PCM

system.cpu = [DerivO3CPU(cpu_id = i) for i in range(0, np)]
for i in range(0, np):
    system.cpu[i].max_insts_all_threads = MAX_INSTS

    # Setup caches for each CPU
    system.cpu[i].icache = L1ICache()
    system.cpu[i].dcache = L1DCache()

    system.cpu[i].icache_port = system.cpu[i].icache.cpu_side
    system.cpu[i].dcache_port = system.cpu[i].dcache.cpu_side

    system.cpu[i].l2bus = L2XBar()
    
    system.cpu[i].icache.mem_side = system.cpu[i].l2bus.slave
    system.cpu[i].dcache.mem_side = system.cpu[i].l2bus.slave

    system.cpu[i].l2cache = L2Cache()
    system.cpu[i].l2bus.master = system.cpu[i].l2cache.cpu_side

    system.cpu[i].l2cache.mem_side = system.l3bus.slave

    # interrupt controller
    system.cpu[i].createInterruptController()

    # Assign workloads
    system.cpu[i].workload = multiprocesses[i]
    system.cpu[i].createThreads()

# Create a PCM memory system
system.hybrid = HybridController()
system.hybrid.range = system.mem_ranges[0]
system.hybrid.cpu_side_port = system.membus.master

# Connect the system up to the membus
system.system_port = system.membus.slave

# set up the root SimObject and start the simulation
root = Root(full_system = False, system = system)

# instantiate all of the objects we've created above
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
