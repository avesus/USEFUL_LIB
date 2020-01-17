#! /usr/bin/env python3


import os
import sys
import argparse

# this is basic driver for testing correctness/performance of all
# sort functions in sequantial_sorting/

parser = argparse.ArgumentParser(description='driver for the driver...')
parser.add_argument("-v", "--verbosity", action="count", default=0, help="increase output verbosity")
parser.add_argument("-c", "--correct", default="", action="store_true", help="correctness check")
parser.add_argument("-m", "--memcheck", default="", action="store_true", help="memory leak check")
parser.add_argument("--s1", default="", help="first to run")
parser.add_argument("--s2", default="", help="second to run")
parser.add_argument("--s3", default="", help="third to run")
parser.add_argument("-l", "--length", default="", help="length to sort")
parser.add_argument("--char", default="", action="store_true", help="type to sort")
parser.add_argument("--short", default="", action="store_true", help="type to sort")
parser.add_argument("--int", default="", action="store_true", help="type to sort")
parser.add_argument("--long", default="", action="store_true", help="type to sort")
parser.add_argument("--str", default="", action="store_true", help="type to sort")
parser.add_argument("--bytes", default="", action="store_true", help="type to sort")
parser.add_argument("--perf", default="", action="store_true", help="type to sort")


flags = parser.parse_args()
verbose = flags.verbosity

regtemp = ""
if flags.perf is True:
    regtemp = "--regtemp"
    
memcheck = ""
if flags.memcheck is True:
    memcheck = "valgrind"

correct = ""
if flags.correct is True:
    correct = "--correct"


length = flags.length

funs = []
funs.append(flags.s1)
funs.append(flags.s2)
funs.append(flags.s3)

types = []
if flags.char is True:
    types.append("--char")
if flags.short is True:
    types.append("--short")
if flags.int is True:
    types.append("--int")
if flags.long is True:
    types.append("--long")
if flags.str is True:
    types.append("--str")
if flags.bytes is True:
    types.append("--bytes")
    
run_format = "(cd {}; make clean; make; {} ./driver --csv --trials 1 --len {}"
for i in funs:
    if i == "":
        continue
    to_run = run_format.format(i, memcheck, length)
    for t in types:
        to_run += " " + t
    to_run += " " + correct
    to_run += " " + regtemp
    to_run += ")"
    print(to_run)
    os.system(to_run)


















