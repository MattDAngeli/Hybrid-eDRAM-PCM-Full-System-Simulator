#!/usr/bin/env python
from os import listdir
from os.path import exists, join

from optparse import OptionParser
parser = OptionParser()
parser.add_option('--dir')
(options, args) = parser.parse_args()

BASE_DIR = options.dir
cfgs=[join(BASE_DIR, f) for f in listdir(BASE_DIR) if f != "process.py"]
for cfg in cfgs:
    fd=open(cfg, "r")
    contents=fd.readlines()
    outputs=[]
    for line in contents:
        outputs.append(line)
        if "parser.add_option('--workload_dir', help=\"workload directory\")" in line:
            outputs.append("parser.add_option('--eDRAM_cache_size', help=\"eDRAM cache size\")\n")
            outputs.append("parser.add_option(\"-w\", action=\"store_true\",\\\n")
            outputs.append("                  dest=\"eDRAM_cache_write_only_mode\",\n")
            outputs.append("                  help=\"eDRAM cache will cache writes only\")\n")
            outputs.append("parser.add_option(\"-n\", action=\"store_false\",\\\n")
            outputs.append("                  dest=\"eDRAM_cache_write_only_mode\",\n")
            outputs.append("                  help=\"eDRAM cache will cache both reads and writes (normal)\")\n")
            outputs.append("parser.add_option('--eDRAM_cache_read_partition', help=\"How much eDRAM for reads\")\n")
            outputs.append("parser.add_option('--eDRAM_cache_write_partition', help=\"How much eDRAM for writes\")\n")
        if "system.hybrid = HybridController()" in line:
            outputs.append("system.hybrid.eDRAM_cache_size = options.eDRAM_cache_size\n")
            outputs.append("system.hybrid.eDRAM_cache_write_only_mode = options.eDRAM_cache_write_only_mode\n")
            outputs.append("system.hybrid.eDRAM_cache_read_partition = options.eDRAM_cache_read_partition\n")
            outputs.append("system.hybrid.eDRAM_cache_write_partition = options.eDRAM_cache_write_partition\n")

    fd.close
    fd=open(cfg, "w")
    fd.writelines(outputs)
        

