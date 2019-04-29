#!/usr/bin/env python

# import the parser
from optparse import OptionParser
parser = OptionParser()
parser.add_option('--out_dir', help="out directory")
(options, args) = parser.parse_args()

from os.path import join

import os
import random

benchmark_cmd=dict()
prog_dir=dict()

np=12 # Number of processors

# mp_dir - where you want to put the generated mp workloads
mp_dir = options.out_dir

def gen_mp(num_unique_benchmarks, num_runs):
    copies_per_benchmark=np/num_unique_benchmarks
    benchmarks=[]
    for key in benchmark_cmd:
        benchmarks.append(key)

    sub_dir=join(mp_dir, str(num_unique_benchmarks))
    if not os.path.exists(sub_dir):
        os.mkdir(sub_dir)
        
    for x in range(0, num_runs):
        # generate random unique benchmarks
        rand_idx=random.sample(range(0, len(benchmarks)), num_unique_benchmarks)

        mp=sub_dir+"/"
        for i in rand_idx[:-1]:
            mp+=benchmarks[i]+"_"
        mp+=benchmarks[rand_idx[-1]]+".py"
        
        # create config file
        mp_config=open(mp, "w+")
        template_part_1_d=open("template_part_1", "r")
        template_part_1=template_part_1_d.readlines()
        template_part_2_d=open("template_part_2", "r")
        template_part_2=template_part_2_d.readlines()

        mp_config.writelines(template_part_1)

        pid=100
        auto_gen_lines=[]
        for i in rand_idx:
            for j in range(0, copies_per_benchmark):
                benchmark=benchmarks[i]
                auto_gen_lines.append("process = Process(pid = " + str(pid) + ")\n")
                auto_gen_lines.append(prog_dir[benchmark] + "\n")
                auto_gen_lines.append("process.cmd = [")
                for arg in benchmark_cmd[benchmark][:-1]:
                    if "prog_dir" in arg:
                        auto_gen_lines.append(arg + ", ")
                    else:
                        auto_gen_lines.append("\"" + arg + "\", ")
                last_arg = benchmark_cmd[benchmark][-1]
                if "prog_dir" in last_arg:
                    auto_gen_lines.append(last_arg + "]\n")
                else:
                    auto_gen_lines.append("\"" + last_arg + "\"]\n")
                auto_gen_lines.append("multiprocesses.append(process)\n\n")
                pid=pid+1

        auto_gen_lines.append("\n")
        mp_config.writelines(auto_gen_lines)
        mp_config.writelines(template_part_2)

prog_dir["perlbench"] = "prog_dir = join(options.workload_dir, \"500.perlbench_r/\")"
benchmark_cmd["perlbench"]=["prog_dir + \"perlbench_r\"", "\"-I\" + prog_dir + \"lib\"", "prog_dir + \"splitmail.pl\"", "6400", "12", "26", "16", "100", "0"]

prog_dir["gcc"] = "prog_dir = join(options.workload_dir, \"502.gcc_r/\")"
benchmark_cmd["gcc"]=["prog_dir + \"cpugcc_r\"", "prog_dir + \"ref32.c\"", "-O3", "-fselective-scheduling", "-fselective-scheduling2", "-o", "prog_dir + \"out\""]

prog_dir["mcf"] = "prog_dir = join(options.workload_dir, \"505.mcf_r/\")"
benchmark_cmd["mcf"]=["prog_dir + \"mcf_r\"", "prog_dir + \"inp.in\""]

prog_dir["namd"] = "prog_dir = join(options.workload_dir, \"508.namd_r/\")"
benchmark_cmd["namd"]=["prog_dir + \"namd_r\"", "--input", "prog_dir + \"apoa1.input\"", "--output", "prog_dir + \"apoa1.ref.output\"", "--iterations", "65"]

prog_dir["parest"] = "prog_dir = join(options.workload_dir, \"510.parest_r/\")"
benchmark_cmd["parest"]=["prog_dir + \"parest_r\"", "prog_dir + \"ref.prm\""]

prog_dir["povray"] = "prog_dir = join(options.workload_dir, \"511.povray_r/\")"
benchmark_cmd["povray"]=["prog_dir + \"povray_r\"", "prog_dir + \"SPEC-benchmark-ref.ini\""]

prog_dir["lbm"] = "prog_dir = join(options.workload_dir, \"519.lbm_r/\")"
benchmark_cmd["lbm"]=["prog_dir + \"lbm_r\"", "3000", "reference.dat", "0", "0", "prog_dir + \"100_100_130_ldc.of\""]

prog_dir["xalancbmk"] = "prog_dir = join(options.workload_dir, \"523.xalancbmk_r/\")"
benchmark_cmd["xalancbmk"]=["prog_dir + \"cpuxalan_r\"", "-v", "prog_dir + \"t5.xml\"", "prog_dir + \"xalanc.xsl\""]

prog_dir["deepsjeng"] = "prog_dir = join(options.workload_dir, \"531.deepsjeng_r/\")"
benchmark_cmd["deepsjeng"]=["prog_dir + \"deepsjeng_r\"", "prog_dir + \"ref.txt\""]

prog_dir["imagick"] = "prog_dir = join(options.workload_dir, \"538.imagick_r/\")"
benchmark_cmd["imagick"]=["prog_dir + \"imagick_r\"", "-limit", "disk", "0", "prog_dir + \"refrate_input.tga\"", "-edge", "41", "-resample", "181%", "-emboss", "31", "-colorspace", "YUV", "-mean-shift", "19x19+15%", "-resize", "30%", "prog_dir + \"refrate_output.tga\""]

prog_dir["leela"] = "prog_dir = join(options.workload_dir, \"541.leela_r/\")"
benchmark_cmd["leela"]=["prog_dir + \"leela_r\"", "prog_dir + \"ref.sgf\""]

prog_dir["nab"] = "prog_dir = join(options.workload_dir, \"544.nab_r/\")"
benchmark_cmd["nab"]=["prog_dir + \"nab_r\"", "1am0", "1122214447", "122"]

prog_dir["xz"] = "prog_dir = join(options.workload_dir, \"557.xz_r/\")"
benchmark_cmd["xz"]=["prog_dir + \"xz_r\"", "prog_dir + \"input.combined.xz\"", "250", "a841f68f38572a49d86226b7ff5baeb31bd19dc637a922a972b2e6d1257a890f6a544ecab967c313e370478c74f760eb229d4eef8a8d2836d233d3e9dd1430bf", "40401484", "41217675", "7"]

gen_mp(4,20)

