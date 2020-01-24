#! /usr/bin/env python3

import os
import argparse


def recursive_clean(dpath):
    if verbose:
        print("Checking: " + dpath)
    files = os.listdir(dpath)
    if "Makefile" in files:
        if verbose:
            print("makefile cleaning: " + dpath)
        os.system("(cd " + dpath + "; make clean)")
    if cleanup:
        if verbose:
            print("cleanup.sh cleaning: " + dpath)
        os.system("(cd " + dpath + "; cleanup.sh)")
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

parser.add_argument("-c", "--cleanup",
                    action="store_true",
                    help="will do full cleanup")


flags = parser.parse_args()
verbose = flags.verbosity
path = flags.path
cleanup = flags.cleanup

recursive_clean(path)
