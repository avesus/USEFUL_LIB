#! /usr/bin/env python3

import os
import argparse

# this is basic driver for testing correctness/performance of all
# sort functions in sequential_hashtable_c

parser = argparse.ArgumentParser(description='driver for the driver...')
parser.add_argument("-v", "--verbosity",
                    action="count",
                    default=0,
                    help="increase output verbosity")
parser.add_argument("-c", "--correct",
                    default="",
                    action="store_true",
                    help="correctness check")
parser.add_argument("-m",
                    "--memcheck",
                    default="",
                    action="store_true",
                    help="memory leak check")
parser.add_argument("-i",
                    "--isize",
                    default="",
                    help="length to sort")
parser.add_argument("-t",
                    "--trials",
                    default="1",
                    help="number of trials")
parser.add_argument("--inserts",
                    default="20",
                    help="num inserts")
parser.add_argument("--queries",
                    default="4",
                    help="queries per insert")
parser.add_argument("--int",
                    default="",
                    action="store_true",
                    help="type for hashtable")
parser.add_argument("--str",
                    default="",
                    action="store_true",
                    help="type for hashtable")
parser.add_argument("--str_nolen",
                    default="",
                    action="store_true",
                    help="type for hashtable")
parser.add_argument("--str_hblen",
                    default="",
                    action="store_true",
                    help="type for hashtable")
parser.add_argument("--perf",
                    default="",
                    action="store_true",
                    help="perfromance test")
parser.add_argument("--alt",
                    default="",
                    action="store_true",
                    help="do alt table")
parser.add_argument("--threads",
                    default="1",
                    help="do builtin table")

flags = parser.parse_args()
verbose = flags.verbosity


inserts = flags.inserts
queries = flags.queries
nthreads = flags.threads
ntrials = flags.trials

regtemp = ""
if flags.perf is True:
    regtemp = "--regtemp"

memcheck = ""
if flags.memcheck is True:
    memcheck = "valgrind"

correct = ""
if flags.correct is True:
    correct = "--correct"

length = flags.isize

funs = []
if flags.alt is True:
    funs.append("alt_hashtable")


types = []
if flags.int is True:
    types.append("int_test")
if flags.str is True:
    types.append("str_test")
if flags.str_nolen is True:
    types.append("str_nolen_test")
if flags.str_hblen is True:
    if verbose > 0:
        print("setting str_hblen")
    types.append("str_hblen_test")


def fixFile(type_test, fpath):
    f_arr = []
    config_file = open(fpath)
    for lines in config_file:
        if "test" in lines:
            if type_test in lines:
                if "//" in lines:
                    lines = lines.replace("//", "")
                f_arr.append(lines)
            else:
                if "//" not in lines:
                    lines = "//" + lines
                f_arr.append(lines)
        else:
            f_arr.append(lines)
    config_file.close()
    write_file = open(fpath, "w")
    for lines in f_arr:
        write_file.write(lines)


config_file_path = ""
run_format = "(cd {}; make clean; make; {} ./driver --csv -t {} --trials {} --isize {} --inserts {} --queries {}"
for i in funs:
    for j in types:
        config_file_path = i + "/test_config.h"
        fixFile(j, config_file_path)
        to_run = run_format.format(i, memcheck, nthreads, ntrials, length, inserts, queries)
        to_run += " " + regtemp
        to_run += " " + correct
        to_run += ")"
        print(to_run)
        os.system(to_run)
