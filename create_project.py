#! /usr/bin/env python3

import os
import sys
import argparse

parser = argparse.ArgumentParser(description='generates new c/cpp project with helper functions and makefile')
parser.add_argument("-v", "--verbosity", action="count", default=0, help="increase output verbosity")
parser.add_argument("-t", "--ftype", default="", help="set type of project as c or cpp")
parser.add_argument("-p", "--path", default="./", help="path to store file in (optional)")
parser.add_argument("-n", "--name", default="", help="name of project")
parser.add_argument("-l", "--lib", default="/home/noah/programs/USEFUL_LIB/", help="name of project")
parser.add_argument("-o", "--overwrite", action="count", default=0, help="write over existing dir")

flags = parser.parse_args()
verbose = flags.verbosity
project_type = flags.ftype
path = flags.path
name = flags.name
overwrite = flags.overwrite
lib_dir = flags.lib

if project_type != "cpp" and project_type != "c":
    print("Error: Invalid File Type!")
    sys.exit(-1)

if name == "":
    print("Error: Invalid Name!")
    sys.exit(-1)

base_path = path + "/" + name
if not os.path.isdir(base_path):
    if verbose:
        print("mkdir " + base_path)
    ret = os.system("mkdir " + base_path)
    if ret != 0:
        print("Error: cant create project: " + base_path)
        exit(-1)
elif overwrite != 0:
    if verbose:
        print("rm -rf " + base_path)
    ret = os.system("rm -rf " + base_path)
    if ret != 0:
        print("Error: cant remove dir: " + base_path)
        exit(-1)
    if verbose:
        print("mkdir " + base_path)
    ret = os.system("mkdir " + base_path)
    if ret != 0:
        print("Error: cant create project: " + base_path)
        exit(-1)

help_path = "helpers/"
compiler = ""
c_prefix = ""
help_lib = help_path
extra_flags = ""
if project_type == "c":
    compiler = "gcc"
    c_prefix = "CC"
    help_path += "c_helpers/"
else:
    compiler = "g++"
    c_prefix = "CXX"
    extra_flags = " -std=c++11"
    help_path += "cpp_helpers/"


helpers = "#include \"helpers/"
build_obj =  "<obj_name>.o: <file_name>." + project_type + " <file_name>.h\n	$(" + c_prefix + ") $(" + c_prefix + "FLAGS) -c <file_name>." + project_type + " -o <file_name>.o $(LDFLAGS)"

writeup_lines = ["1) Noah Goldstein"]

header_code_lines = ["#include <stdio.h>",
                     "#include <stdlib.h>",
                     "#include <pthread.h>",
                     "#include <assert.h>",
                     "#include <string.h>"]
if project_type == "cpp":
    header_code_lines.append("#include <string>")
    header_code_lines.append("#include <sstream>")
    header_code_lines.append("#include <vector>")
    header_code_lines.append("#include <iostream>")
    header_code_lines.append("#include <thread>")
    header_code_lines.append("#include <mutex>")
    header_code_lines.append("#include <iterator>")
    header_code_lines.append("#include <map>")



source_code_lines = ["#include \"<file_name>.h\"",
                     "",
                     "",
                     "#define SUCCESS 0",
                     "#define FAILURE -1",
                     "",
                     "",
                     "int verbose = 0;",
                     "",
                     "#define Version \"0.1\"",
                     "",
                     "static ArgOption args[] = {",
                     "  // Kind, 	  Method,		name,	    reqd,  variable,		help",
                     "  { KindOption,   Integer, 		\"-v\", 	    0,     &verbose, 		\"Set verbosity level\" },",
                     "  { KindHelp,     Help, 	\"-h\" },",
                     "  { KindEnd }",
                     "};",
                     "static ArgDefs argp = { args, \"" + name + "\", Version, NULL };",
                     "",
                     "using namespace std;",
                     "",
                     "int main(int argc, char* argv[]){",
                     "  progname = argv[0];",
                     "  ArgParser* ap = createArgumentParser(&argp);",
                     "  int result = parseArguments(ap, argc, argv);",
                     "  if(result){",
                     "    die(\"Error parsing arguments\");",
                     "  }",
                     "  freeCommandLine();",
                     "  freeArgumentParser(ap);",
                     "",
                     "  return SUCCESS;",
                     "}"]


make_file_lines = [c_prefix + "=" + compiler,
                   c_prefix + "FLAGS=-O0 -g -D_GNU_SOURCE" + extra_flags,
                   "LDFLAGS=-lpthread -lm",
                   "",
                   "H_OBJS=",
                   "",
                   "all: <file_name>",
                   ""]


h_objects = ""
obj_lines = []


if verbose:
    print("mkdir " + base_path + "/helpers")
os.system("mkdir " + base_path + "/helpers")
for sfile in os.listdir(lib_dir + help_path):
    if "~" not in sfile and "#" not in sfile:
        if verbose:
            print("cp " + lib_dir + help_path + "/" + sfile + " " + base_path + "/helpers/")
        os.system("cp " + lib_dir + help_path + "/" + sfile + " " + base_path + "/helpers/")
        if ".h" in sfile and "config" not in sfile:
            h_include = helpers + sfile + "\""
            h_str = build_obj.replace("<file_name>", "helpers/" + sfile.strip(".h"))
            h_str = h_str.replace("<obj_name>", sfile.strip(".h"))
            obj_lines.append(h_str)
            h_objects += "helpers/" + sfile.replace(".h", ".o") + " "
            header_code_lines.append(h_include)
        

h_str = build_obj.replace("<file_name>", name)
h_str = h_str.replace("<obj_name>", name)
obj_lines.append(h_str)

for lines in obj_lines:
    make_file_lines.append(lines)
    make_file_lines.append("")

make_file_lines.append("<file_name>: <file_name>.o $(H_OBJS)")
make_file_lines.append("	$("+c_prefix+") $("+c_prefix+"FLAGS) <file_name>.o $(H_OBJS) -o <file_name> $(LDFLAGS)")
make_file_lines.append("")
make_file_lines.append("clean:")
make_file_lines.append("	rm -f <file_name> *~ *.o *#* helpers/*~ helpers/*#* + helpers/*.o")
make_file_lines.append("")


new_makefile = open(base_path + "/Makefile", "w+")
new_source = open(base_path + "/" + name + "." + project_type, "w+")
new_header = open(base_path + "/" + name + ".h", "w+")

format_str = "{}\n"
for lines in make_file_lines:
    proper_line = lines.replace("<file_name>", name)
    proper_line = proper_line.replace("CCFLAGS", "CFLAGS")
    if "H_OBJS=" in proper_line:
        proper_line += h_objects
    new_makefile.write(format_str.format(proper_line))

for lines in source_code_lines:
    proper_line = lines.replace("<file_name>", name)
    proper_line = proper_line.replace("CCFLAGS", "CFLAGS")
    if "namespace" in lines and project_type == "c":
        continue
    new_source.write(format_str.format(proper_line))

for lines in header_code_lines:
    proper_line = lines.replace("<file_name>", name)
    proper_line = proper_line.replace("CCFLAGS", "CFLAGS")
    new_header.write(format_str.format(proper_line))


new_header.close()
new_source.close()
new_makefile.close()
