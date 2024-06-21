#!/bin/bash

# Table 1
python3 bench_compwise.py networks er 5

# Table 2
python3 bench_compwise.py add nawae 5
python3 bench_compwise.py mult nawae 5
python3 bench_compwise.py mac nawae 5

# Table 3
python3 bench asymm er 100