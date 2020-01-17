#!/bin/bash

# there must be NO memory leaks!
./test.py --length 17 --s1 normal_sort --s2 prep_sort --char --short --int --long --correct --memcheck
./test.py --length 17 --s1 prep_sort --str --bytes  --memcheck --correct
