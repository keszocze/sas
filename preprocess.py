import subprocess
import os
import common
import re
import sys

import argparse

parser = argparse.ArgumentParser(description='Pre-process benchmarks for SAS', 
    epilog="""
        There are some implicit default values for the bit sizes:
            - Add:  n=16, N=256, stride=16
            - Mult: n=1, N=13, stride=1
            - Asymm: n=1, N=18, stride=1
            - Mac: n=1, N=5, stride=1

        Note that this script does not check for the existence of pre-processed files and, hence, will happily re-compute everything.""", 
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("type", help="The type of benchmark to pre-process", type=str, choices=['add', 'mult', 'mac', 'asymm', 'networks'])
parser.add_argument("--timeout", help="Timeout for the executed ABC command", type=str, default=common.TIMEOUT)
parser.add_argument("--start", help="Minimal bit size", type=int, metavar='N')
parser.add_argument("--end", help="Maximal bit size", type=int, metavar='N')
parser.add_argument("--stride", help="Stride for increasing the bit size", metavar='N', type=int)
parser.add_argument("--start-pairs", help="Minimal number of pairs for the 'mac' benchmarks", type=int, default=2, metavar='P')
parser.add_argument("--end-pairs", help="Maximal number of pairs for the 'mac' benchmarks", type=int, default=5, metavar='P')
parser.add_argument("--optimize-command", help="ABC command to optimize the circuits", metavar="CMD", type=str, default=common.OPTIMIZE_COMMAND)
args=parser.parse_args()

# the type of benchmark
btype = args.type


result = "name;t_opt;n_aig;t_gbdd;n_bdd\n"


def preprocess(read_command, name, category):
    print("Preprocessing " + name + "...")
    global result

    out_folder = "benchmark/preprocessed/" + category
    os.makedirs(out_folder, exist_ok=True)
    out_basename = out_folder + "/" + name

    abc_command = "source ./../../abc.rc\n" \
                  + read_command + "\n" \
                  + "time\n" \
                  + common.OPTIMIZE_COMMAND + "\n" \
                  + "time\n" \
                    "gbdd_build 1\n" \
                    "time\n" \
                    "print_stats\n" \
                    "write " + out_basename + ".aig\n" \
                  + "gbdd_store " + out_basename + ".bdd\n"
    #print(abc_command)
    proc_result = subprocess.run(["timeout", args.timeout, "./../../abc"],
                                 input=abc_command.encode('utf-8'),
                                 capture_output=True)
    if proc_result.returncode == 124:
        print("Timed out!")
        return False
    #else:
    #    print(proc_result.stdout)
    commands = common.stdout_to_lines(proc_result.stdout)
    t_opt = common.read_time_command(commands[4])
    bdd_count = re.findall(r"Node count: (\d+)", commands[5][2])[0]
    t_bdd = common.read_time_command(commands[6])
    and_count = common.get_ands(commands[7])
    result = result + name + ";" + t_opt + ";" + and_count + ";" + t_bdd + ";" + bdd_count + "\n"
    return True








def setup_loop(default_start, default_end, default_stride):
    if args.start:
        n=args.start
    else:
        n=default_start

    if args.end:
        N=args.end
    else:
        N=default_end

    if args.stride:
        stride=args.stride
    else:
        stride=default_stride
    
    return n, N, stride


match args.type:
    case 'add':
        n, N, stride = setup_loop(16,256,16)
        while n <= N and preprocess("netgen adder " + str(n), "add" + str(n), "add") :
            n += stride
    case 'mult':
        n, N, stride = setup_loop(1,13,1)
        while n <= N and preprocess("netgen multiplier " + str(n), "mult" + str(n), "mult"):
            n += stride

    case 'asymm':
        n, N, stride = setup_loop(1,18,1)
        while n <= N and preprocess("netgen asymmetric " + str(n) + " 1", "asymm" + str(n), "asymm"):
            n += stride

    case "networks":
        for filename in os.listdir("benchmark/networks"):
            if filename.endswith('.pla'):
                preprocess("read benchmark/networks/" + filename, filename.split(".")[0], "networks")
    
    case "mac":
        n_init, N, stride = setup_loop(1,5,1)
        for n_pairs in range(args.start_pairs, args.end_pairs + 1):
            n=n_init
            while n <= N and preprocess("netgen mac " + str(n) + " " + str(n_pairs),
                                "mac" + str(n_pairs) + "x" + str(n),
                                "mac"):
                n += stride


log_file = open("benchmark/preprocessed/" + btype + "/log.csv", "w")
log_file.write(result)
log_file.close()
