import os

import common
import subprocess
import re
import sys
from datetime import datetime
import argparse



parser = argparse.ArgumentParser(description='Run component-wiste benchmark', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("type", help="The benchmark set to use", type=str, choices=['add', 'mult', 'mac', 'asymm', 'networks'])
parser.add_argument("metric", help="The error metric to use", type=str, choices=['er', 'awae','nawae'])
parser.add_argument("ErrorThreshold", help="The error threshold to use", type=str)
parser.add_argument("-v", "--verbose", help="Prints more information (more 'v' more output)", action="count", default=0)
parser.add_argument("--timeout", help="Timeout for the executed ABC command", type=str, default=common.TIMEOUT)
parser.add_argument("--optimize-command", help="ABC command to optimize the circuits", metavar="CMD", type=str, default=common.OPTIMIZE_COMMAND)
args=parser.parse_args()

# the type of benchmark
btype = args.type
error_metric=args.metric
threshold=args.ErrorThreshold


header = "time;name;error_metric;threshold;pi;po;n_bdd;n_aig;t_symm;t_aig;t_bdd;"
for b in ["ub", "b"]:
    for opt in ["const", "bdd", "aig"]:
        v = b + "_" + opt + "_"
        header += v + "t_select;" + v + "selection;" \
                  + v + "n_bdd;" + v + "n_bdd_gain;" \
                  + v + "n_aig;" + v + "n_aig_gain;" \
                  + v + "err;"

dir = "benchmark/compwise/" + btype + "/"
os.makedirs(dir, exist_ok=True)

now = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
filename = f"{dir}{now}_{btype}_{error_metric}_{threshold}.csv"
out_file = open(filename, "w")
out_file.write(header + "\n")

def symmetrize_t(basename, category, error_metric, optimization_target, error_bound=threshold,
                 additional_info=False):
    preprocessed_base = "benchmark/preprocessed/" + category + "/" + basename
    abc_command = "source ./../../abc.rc\n" \
                  + "read " + preprocessed_base + ".aig\n" \
                  + "gbdd_load " + preprocessed_base + ".bdd\n" \
                  + "print_io\n" \
                  + "symmetrize " + error_metric + " " + error_bound + " " + optimization_target \
                  + " \"" + args.optimize_command + "\"\n"
    proc_result = subprocess.run(["timeout", args.timeout, "./../../abc"],
                                 input=abc_command.encode('utf-8'),
                                 capture_output=True)

    if args.verbose >= 2:
        print(f"ACB command: {abc_command}")


    if proc_result.returncode == 124:
        print("Timed out!")
        return ("-;-;-;-;-;-;-;" if additional_info else "") + "-;-;-;-;-;-;-;"

    commands = common.stdout_to_lines(proc_result.stdout)
    symm_command = commands[4]
    io_command = commands[3]

  

    t_symm = common.read_time(symm_command[1])
    t_aig = common.read_time(symm_command[3])
    t_bdd = common.read_time(symm_command[4])
    t_select = common.read_time(symm_command[5])

    if args.verbose >= 3:
        print(f"Symm command: {symm_command}")

    aig_size = re.findall(r"AIG size: (\d+) -> (\d+) \((([-\d.%]|inf|nan)+)\)", symm_command[7])[0]
    bdd_size = re.findall(r"BDD size: (\d+) -> (\d+) \((([-\d.%]|inf|nan)+)\)", symm_command[8])[0]
    selection = re.findall(r"Selection: ([01]+)", symm_command[9])[0]
    error = re.findall(r"Total error: (\d*\.\d*)", symm_command[10])[0]

    pis = re.findall(r"^Primary inputs \((\d+)\):",io_command[1])[0]
    pos = re.findall(r"^Primary outputs \((\d+)\):",io_command[2])[0]

    additional = ";".join([pis, pos, bdd_size[0], aig_size[0], t_symm, t_aig, t_bdd]) + ";"
    return (additional if additional_info else "") \
        + ";".join([
            t_select, selection,
            bdd_size[1], bdd_size[2],
            aig_size[1], aig_size[2],
            error]) + ";"


def symmetrize(basename, category, error_metric, T):
    now = datetime.now().strftime("%H:%M:%S %d.%m.%Y")
    print(f"\n[{now}] Symmetrizing " + basename + "...")
    result = now + ";" + basename +  ";" + error_metric + ";" + threshold + ";" \
              + symmetrize_t(basename, category, error_metric, "const", "100", True) \
              + symmetrize_t(basename, category, error_metric, "bdd", "100") \
              + symmetrize_t(basename, category, error_metric, "aig", "100") \
              + symmetrize_t(basename, category, error_metric, "const", threshold) \
              + symmetrize_t(basename, category, error_metric, "bdd", threshold) \
              + symmetrize_t(basename, category, error_metric, "aig", threshold) 
    if args.verbose >= 1:
        print(result)
    out_file.write(result + "\n")


for file in os.listdir("benchmark/preprocessed/" + btype):
    if file.endswith(".aig"):
        symmetrize(os.path.basename(os.path.splitext(file)[0]), btype, error_metric, threshold)


out_file.close()
