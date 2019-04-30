#!/usr/bin/env python

from os import listdir, mkdir
from os.path import exists, join
import subprocess

BASE_DIR="configs/simulation/CPU2017_MP"

OUT_DIR="CPU2017_MP_Assessment"

if not exists(OUT_DIR):
    mkdir(OUT_DIR)

def processM5out(run_name,num_unique_benchmarks):
    fd=open("m5out/stats.txt", "r")
    stats=fd.readlines()
    
    outF=join(OUT_DIR,num_unique_benchmarks)
    outD=open(outF, "a")

    results = [] # final prstring elements

    read_queue_info = []
    write_queue_info = []
    for line in stats:
        if "average_read_queue_size" in line:
            read_queue_info.append(float(line.split()[1]))
        if "average_write_queue_size" in line:
            write_queue_info.append(float(line.split()[1]))

        if "num_read_allos" in line:
            num_read_allos = str(line.split()[1])
        if "num_write_allos" in line:
            num_write_allos = str(line.split()[1])
        if "num_edram_evics" in line:
            num_edram_evics = str(line.split()[1])
        if "num_of_write_hits" in line:
            num_of_write_hits = str(line.split()[1])
        if "num_of_read_hits" in line:
            num_of_read_hits = str(line.split()[1])
        if "num_of_wb_hits" in line:
            num_of_wb_hits = str(line.split()[1])

        if "num_of_pcm_reads" in line:
            num_of_pcm_reads = str(line.split()[1])
        if "num_of_pcm_writes" in line:
            num_of_pcm_writes = str(line.split()[1])
        if "num_diff_writes" in line:
            num_diff_writes = str(line.split()[1])
        if "num_same_writes" in line:
            num_same_writes = str(line.split()[1])

    results.append(str(max(read_queue_info)))
    results.append(str(max(write_queue_info)))
    results.append(num_read_allos)
    results.append(num_write_allos)
    results.append(num_edram_evics)
    results.append(num_of_write_hits)
    results.append(num_of_read_hits)
    results.append(num_of_wb_hits)
    results.append(num_of_pcm_reads)
    results.append(num_of_pcm_writes)
    results.append(num_diff_writes)
    results.append(num_same_writes)

    if len(results) != 0:
        print_str = run_name+","
        for result in results[:-1]:
            print_str += result
            print_str += ","
        print_str += results[-1]
        print_str += "\n"
        outD.write(print_str)

    outD.close()

def runSimulation():
    simulation_dirs=[join(BASE_DIR, d) for d in listdir(BASE_DIR) if d != "gen_mp"]
    
    for d in simulation_dirs:
        runs=[join(d, f) for f in listdir(d)]
        
        num_unique_benchmarks=d.split("/")[-1]
        for run in runs:
            subprocess.call(["build/ARM/gem5.opt", "--debug-flags=PCMSim", run, "--workload_dir", "/home/shihao-song/Documents/Research_Extend_1/CPU2017_ARM/"])
            run_name=run.split("/")[-1]
            processM5out(run_name,num_unique_benchmarks)

runSimulation()
