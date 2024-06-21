#!/bin/bash

# beware, this script will take some time to finish

python3 preprocess.py --start 16 --end 256 --stride 16 add
python3 preprocess.py --start 1 --end 13 --stride 1 mult
python3 preprocess.py networks
python3 preprocess.py --start 1 --end 5 --stride 1 --start-pairs 2 --end-pairs 3 mac
python3 preprocess.py --start 1 --end 4 --stride 1 --start-pairs 4 --end-pairs 4 mac
python3 preprocess.py --start 1 --end 3 --stride 1 --start-pairs 5 --end-pairs 5 mac
python3 preprocess.py --start 1 --end 18 --stride 1 asymm