# SAS - A Framework for Symmetry-based Approximate Synthesis

SAS is a framework for symmetry-based approximate synthesis. For deatils see the [open access paper](https://doi.org/10.1145/3649329.3658495) presented at [DAC'24](https://www.dac.com/).

## Installation

SAS is build in top of [ABC](https://github.com/berkeley-abc/abc) and written in C++. Hence, you need ABC and the means to compile C/C++ code.

#### 1 Installing ABC
Install ABC in a folder of your choice. Do not build it yet.

```
cd <path/where/you/want/to/install/abc/to>
git clone https://github.com/berkeley-abc/abc.git

```


#### 2 Add SAS to ABC
Clone this repository into `src/ext-symmetrize` via
```bash
cd abc/src
git clone https://github.com/keszocze/sas ext-sas
```

**Note**: It is important to clone the repository in the `ext-sas` folder (and not `sas` only). The `ext` prefix allows ABC to find the SAS code.

#### 3 Build SAS/ABC
```bash
cd ..
make
<wait for quite some time>
```
After a succefsull build, the last printed line should be
```
`` Building binary: abc
```

#### 4 Try it
You can verify that SAS has been properly compiled and can be used within ABC as follows
```bash
./abc
netgen adder 10
gbdd_build 0
symmetrize er 25 const
```
This is for the interactive use within the ABC repl. Running full benchmarks is described below.


## Run benchmarks
Before running the SAS synthesis, go into the `abc/src/ext-sas/` folder. First you need to create the AIGs/BDDs for the benchmarks:
```bash
# Generate AIGs and global BDDs once for each benchmark category (takes time!)
python3 preprocess.py [add/mult/mac/asymm/networks]
```
(also check the options for the script, see below)

Then to actually run the benchmarks, do the following:
```
# Run componentwise symmetrization benchmarks for each category
python3 bench_compwise.py [add/mult/mac/asymm/networks] [error metric: er/awae/nawae] [error threshold]
```



To reproduce the results of da DAC'24 paper, run the following scripts
```
dac_preprocess.sh
dac_benchmark.sh
```
Note: this will take quite some time.

## Scripts

### `preprocess.py` 
This file provides the means to generate the arithmetic (add, mul, mac) and maximally asymmetric benchmarks. It also copies networks taken from [EPFL Benchmark Suite](https://www.epfl.ch/labs/lsi/page-102566-en-html/benchmarks/) and the ESPRESSO tool.

 After the benchmarks have been generated/copied, the script also optimizes the networks. This allows to have a fair starting point for the evaluation of the reductions and to allow for comparisons against SAS as this is also creates a well defined data set. As this takes quite some time you might want to skip this step when it is not strictly required. Note, though, that it only needs to done once.

 ```
 Pre-process benchmarks for SAS

positional arguments:
  {add,mult,mac,asymm,networks}
                        The type of benchmark to pre-process

options:
  -h, --help            show this help message and exit
  --timeout TIMEOUT     Timeout for the executed ABC command (default: 4h)
  --start N             Minimal bit size (default: None)
  --end N               Maximal bit size (default: None)
  --stride N            Stride for increasing the bit size (default: None)
  --start-pairs P       Minimal number of pairs for the 'mac' benchmarks (default: 2)
  --end-pairs P         Maximal number of pairs for the 'mac' benchmarks (default: 5)
  --optimize-command CMD
                        ABC command to optimize the circuits (default: runsc resyn2)

There are some implicit default values for the bit sizes: 
  - Add: n=16, N=256, stride=16 
  - Mult: n=1, N=13, stride=1 
  - Asymm: n=1, N=18, stride=1 
  - Mac: n=1, N=5, stride=1 
  
Note that this script does not check for the existence of pre-processed files and, hence, will happily re-compute everything.
```

### `bench_compwise.py`
This script carries out the synthesis on the benchmark files generated by `preprocess.py` and stores the results in the `benchmark/compwise/<BENCH TYPE>` folder.
Note that the script always runs an unbounded synthesis as well as one with the provided error threshold.

The usage is straight-forward:
```
usage: bench_compwise.py [-h] [-v] [--timeout TIMEOUT] [--optimize-command CMD] {add,mult,mac,asymm,networks} {er,awae,nawae} ErrorThreshold

Run component-wiste benchmark

positional arguments:
  {add,mult,mac,asymm,networks}
                        The benchmark set to use
  {er,awae,nawae}       The error metric to use
  ErrorThreshold        The error threshold to use

options:
  -h, --help            show this help message and exit
  -v, --verbose         Prints more information (more 'v' more output) (default: 0)
  --timeout TIMEOUT     Timeout for the executed ABC command (default: 4h)
  --optimize-command CMD
                        ABC command to optimize the circuits (default: runsc resyn2)
```


### `common.py`
This file stores the default values for the timout (4 hours) and the optimization command to run (`runsc resyn2`). Both values can be overriden by the corroesponding command line options of `preprocess.py` and `bench_compwise.py`.


This file als contains some common utility code for parsing ABC output.

Most likely, you never need to use/edit this script directly.

## Citing SAS
If you are using SAS or referencing it in your publication, please cite the following paper:

 >Niklas Jungnitz and Oliver Keszocze. 2024. “SAS - A Framework for Symmetry-Based Approximate Synthesis.” In Design Automation Conference. DAC’24. San Francisco, CA, USA. https://doi.org/10.1145/3649329.3658495.


```Bibtex
@inproceedings{Jungnitz2024,
  title = {{{SAS}} - {{A Framework}} for {{Symmetry-based Approximate Synthesis}}},
  booktitle = {Design Automation Conference},
  author = {Jungnitz, Niklas and Keszocze, Oliver},
  date = {2024-06},
  series = {{{DAC}}'24},
  location = {San Francisco, CA, USA},
  doi = {10.1145/3649329.3658495},
}
```