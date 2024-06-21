import re

TIMEOUT = "4h"
OPTIMIZE_COMMAND = "runsc resyn2"


def stdout_to_lines(stdout):
    stdout: str = stdout.decode("utf-8")
    commands = stdout.split("\nabc")
    for idx, command in enumerate(commands):
        if idx != 0:
            command = command[5:]
        commands[idx] = command.split("\n")
    if not commands[0][0].startswith("UC Berkeley, ABC"):
        raise Exception("expected ABC output")
    return commands[1:]


def get_ands(print_stats_command):
    return re.findall(r"and = +(\d+)", print_stats_command[1])[0]


def read_time_command(time_command):
    return re.findall(r"elapse: ([\d.]+)", time_command[1])[0]


def read_time(time_str):
    return re.findall(r"= *([\d.]+) sec", time_str)[0]
