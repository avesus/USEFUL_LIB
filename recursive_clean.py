#! /usr/bin/env python3

import os
import argparse


def recursive_clean(dpath):
    if verbose:
        print("Checking: " + dpath)
    files = os.listdir(dpath)
    if "Makefile" in files:
        if verbose:
            print("Cleaning: " + dpath)
        os.system("(cd " + dpath + "; make clean)")
    next_calls = []
    for f in files:
        next_path = dpath + "/" + f
        if os.path.isdir(next_path):
            next_calls.append(next_path)
    for d in next_calls:
        if verbose:
            print("Next Call: " + d)
        recursive_clean(d)

        
parser = argparse.ArgumentParser(description='recursive make clean')
parser.add_argument("-v", "--verbosity",
                    action="store_true",
                    help="increase output verbosity")

parser.add_argument("-p", "--path",
                    default=".",
                    help="path to start recursive cleaning from")

flags = parser.parse_args()
verbose = flags.verbosity
path = flags.path

recursive_clean(path)
