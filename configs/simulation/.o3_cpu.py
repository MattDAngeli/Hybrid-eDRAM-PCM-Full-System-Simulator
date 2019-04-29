# import the m5 (gem5) library created when gem5 is built
import m5
# import all of the SimObjects
from m5.objects import *

# import custom caches
from caches import *

# create the system we are going to simulate
system = System()

# Set the clock fequency of the system (and all of its children)
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '3.32GHz'
system.clk_domain.voltage_domain = VoltageDomain()

# Set up the system
system.mem_mode = 'timing'               # Use timing accesses
system.mem_ranges = [AddrRange('8GB')] # Create an address range

# Create a simple CPU
# To create a DerivO3CPU, we need a system with separate instruction
# and data caches
system.cpu = DerivO3CPU()

# Create a memory bus, a system crossbar, in this case
system.membus = SystemXBar()

# Hook the CPU ports to L1 cache
system.cpu.icache = L1ICache()
system.cpu.dcache = L1DCache()

system.cpu.icache_port = system.cpu.icache.cpu_side
system.cpu.dcache_port = system.cpu.dcache.cpu_side

# Hook L1 to L2Cache
system.l2bus = L2XBar()

system.cpu.icache.mem_side = system.l2bus.slave
system.cpu.dcache.mem_side = system.l2bus.slave

system.l2cache = L2Cache()
system.l2bus.master = system.l2cache.cpu_side

# Hook L2 to membus
system.membus = SystemXBar()
system.l2cache.mem_side = system.membus.slave

# Hook L3 (Reference)
# system.l3cache.xbar = L2XBar()
# system.l2cache.mem_side = system.l3cache.xbar.slave
# system.l3cache.cpu_side = system.l3cache.xbar.master
# system.l3cache.mem_side = system.membus.slave

# create the interrupt controller for the CPU and connect to the membus
system.cpu.createInterruptController()

# For x86 only, make sure the interrupts are connected to the memory
# Note: these are directly connected to the memory bus and are not cached
system.cpu.interrupts[0].pio = system.membus.master
system.cpu.interrupts[0].int_master = system.membus.slave
system.cpu.interrupts[0].int_slave = system.membus.master

# Create a DDR3 memory controller and connect it to the membus
system.mem_ctrl = HybridController()
system.mem_ctrl.range = system.mem_ranges[0]
system.mem_ctrl.cpu_side_port = system.membus.master

# Connect the system up to the membus
system.system_port = system.membus.slave

# Create a process for a simple "Hello World" application
process = Process()
# Set the command
# cmd is a list which begins with the executable (like argv)
# prog_dir = 'tests/test-progs/hello_w_args/'
# process.cmd = [prog_dir + "file", prog_dir + "hello.c"]
# process.output = prog_dir + "out"
prog_dir = "/home/shihao-song/Documents/Research/CPU2017/benchspec/CPU/505.mcf_r/run/run_base_refrate_mytest-m64.0000/"
process.cmd = [prog_dir + "mcf_r", prog_dir + "inp.in"]
# process.input = prog_dir + "bwaves_4.in"
# process.output = prog_dir + "bwaves_4.out"
# process.errout = prog_dir + "bwaves_4.err"

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
