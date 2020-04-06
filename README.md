## A Fix-and-Optimize matheuristic applied to the Home Health Care Routing and Scheduling Problem

This repository contains the code of the genetic algorithm presented in [SBPO 2019](sbpo2019.galoa.com.br/).

## Building the project

To be able to compile the code, you need:
- A C++17 compiler (clang >= 6.0 should be fine)
- CMake build system
- CPLEX MIP solver (>=12.8.0 should be fine)

On a updated Debian/Ubuntu/Linux Mint installation, you can install those dependencies
by running `sudo apt install clang++-8 cmake libomp-8-dev`. To other versions of `clang`,
check your distro documentation.

To build the project, browse to the `build` directory and issue the command `CXX=clang++-8 cmake .. -DCPLEX_ROOT_DIR=$CPX` to generate the makefile. Then, simply run `make`. By default, CMake is set to generate a debug-friendly binary, with all code optimizations disabled and all symbols embedded. To change this behavior, run `CXX=clang++-8 cmake .. -DCPLEX_ROOT_DIR=$CPX -DCMAKE_BUILD_TYPE=Release` to enable the `-O3` optimization flag.

__Note:__ You need to specify the path to the `IBM ILOG CPLEX Studio` by replacing the value of `$CPX`. Supposing a standard installation of CPLEX on a Ubuntu 18.04 system and `clang 9`, the entire configuration and build of the project holds the following output.

```bash
$ CXX=clang++-9 cmake -DCPLEX_ROOT_DIR=/opt/ibm/ILOG/CPLEX_Studio128/ ..
-- The C compiler identification is Clang 9.0.0
-- The CXX compiler identification is Clang 9.0.0
-- Check for working C compiler: /usr/bin/clang-9
-- Check for working C compiler: /usr/bin/clang-9 -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/clang++-9
-- Check for working CXX compiler: /usr/bin/clang++-9 -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Using compilation flags of mode "Debug"
-- Using CPLEX_ROOT_DIR = "/opt/ibm/ILOG/CPLEX_Studio128/"
-- Configuring done
-- Generating done
-- Build files have been written to: /home/alberto/work/sbpo2019-fix-and-optimize/build

$ make
Scanning dependencies of target fixAndOptimize
[ 14%] Building CXX object CMakeFiles/fixAndOptimize.dir/src/SolutionCopy.cpp.o
[ 42%] Building CXX object CMakeFiles/fixAndOptimize.dir/src/Instance.cpp.o
[ 42%] Building CXX object CMakeFiles/fixAndOptimize.dir/src/FixAndOptimize.cpp.o
[ 57%] Building CXX object CMakeFiles/fixAndOptimize.dir/src/mainFeo.cpp.o
[ 71%] Building CXX object CMakeFiles/fixAndOptimize.dir/src/InitialRouting.cpp.o
[ 85%] Building CXX object CMakeFiles/fixAndOptimize.dir/src/MipModel.cpp.o
[100%] Linking CXX executable fixAndOptimize
[100%] Built target fixAndOptimize

```

## Running the genetic algorithm

Once compiled, you should be ready to use this implementation of the F&O matheuristic. If you execute the binary `fixAndOptimize` without any arguments, it will present you the command line usage.

```bash
$ ./fixAndOptimize
Usage: ./fixAndOptimize <1:instance path> <2: PRNG seed>
```

Following the list of parameters, you need to specify:

- `<1: instance path>` The path to the instance file to be solved
- `<2: seed>` The seed to be set into the Pseudo-RNG

An example of usage is the following.

```bash
$ ./fixAndOptimize ../instances-HHCRSP/InstanzCPLEX_HCSRP_25_6.txt 1
=== Fix-and-Optimize solver for HHCRSP ===
Instance: ../instances-HHCRSP/InstanzCPLEX_HCSRP_25_6.txt
PRGN seed: 1
Creating MIP model... Done!
Creating initial constructive solution... Done!
Setting solution to MIP model...
Done! Initial solution cost: 1835.53.
Iteration: 1  Decomp: random  Elapsed: 20.6 secs  Obj: 638.7  Improved: 187.4%  IWoI: 0
Iteration: 2  Decomp: random  Elapsed: 20.6 secs  Obj: 638.7  Improved: 0.0%  IWoI: 0
Iteration: 3  Decomp: random  Elapsed: 34.9 secs  Obj: 638.7  Improved: 0.0%  IWoI: 1
Iteration: 4  Decomp: guided  Elapsed: 51.0 secs  Obj: 576.1  Improved: 10.9%  IWoI: 2
Iteration: 5  Decomp: guided  Elapsed: 52.0 secs  Obj: 576.1  Improved: 0.0%  IWoI: 0
Iteration: 6  Decomp: random  Elapsed: 52.0 secs  Obj: 574.2  Improved: 0.3%  IWoI: 1
Iteration: 7  Decomp: random  Elapsed: 52.1 secs  Obj: 570.4  Improved: 0.7%  IWoI: 0
Iteration: 8  Decomp: random  Elapsed: 54.2 secs  Obj: 563.9  Improved: 1.2%  IWoI: 0
Iteration: 9  Decomp: random  Elapsed: 55.4 secs  Obj: 563.9  Improved: 0.0%  IWoI: 0
Iteration: 10  Decomp: guided  Elapsed: 55.5 secs  Obj: 563.9  Improved: 0.0%  IWoI: 1
Iteration: 11  Decomp: guided  Elapsed: 55.5 secs  Obj: 563.9  Improved: 0.0%  IWoI: 2
Iteration: 12  Decomp: guided  Elapsed: 58.9 secs  Obj: 563.9  Improved: 0.0%  IWoI: 3
Iteration: 13  Decomp: random  Elapsed: 58.9 secs  Obj: 563.9  Improved: 0.0%  IWoI: 4
Iteration: 14  Decomp: random  Elapsed: 63.3 secs  Obj: 537.0  Improved: 5.0%  IWoI: 5
Iteration: 15  Decomp: guided  Elapsed: 82.4 secs  Obj: 523.5  Improved: 2.6%  IWoI: 0
Iteration: 16  Decomp: guided  Elapsed: 82.7 secs  Obj: 523.5  Improved: 0.0%  IWoI: 0
Iteration: 17  Decomp: random  Elapsed: 90.9 secs  Obj: 523.5  Improved: 0.0%  IWoI: 1
Iteration: 18  Decomp: random  Elapsed: 91.2 secs  Obj: 523.5  Improved: 0.0%  IWoI: 2
Iteration: 19  Decomp: random  Elapsed: 95.4 secs  Obj: 509.1  Improved: 2.8%  IWoI: 3
Iteration: 20  Decomp: guided  Elapsed: 97.9 secs  Obj: 509.1  Improved: 0.0%  IWoI: 0
Iteration: 21  Decomp: random  Elapsed: 110.0 secs  Obj: 474.9  Improved: 7.2%  IWoI: 1
Iteration: 22  Decomp: guided  Elapsed: 118.1 secs  Obj: 471.4  Improved: 0.7%  IWoI: 0
Iteration: 23  Decomp: guided  Elapsed: 131.4 secs  Obj: 471.4  Improved: 0.0%  IWoI: 0
Iteration: 24  Decomp: guided  Elapsed: 144.7 secs  Obj: 471.4  Improved: 0.0%  IWoI: 1
Iteration: 25  Decomp: random  Elapsed: 151.2 secs  Obj: 471.4  Improved: 0.0%  IWoI: 2
Iteration: 26  Decomp: guided  Elapsed: 153.7 secs  Obj: 471.4  Improved: 0.0%  IWoI: 3
Iteration: 27  Decomp: random  Elapsed: 156.1 secs  Obj: 471.4  Improved: 0.0%  IWoI: 4
Iteration: 28  Decomp: guided  Elapsed: 167.6 secs  Obj: 471.4  Improved: 0.0%  IWoI: 5
Iteration: 29  Decomp: random  Elapsed: 174.2 secs  Obj: 471.4  Improved: 0.0%  IWoI: 6
Iteration: 30  Decomp: guided  Elapsed: 175.9 secs  Obj: 467.8  Improved: 0.8%  IWoI: 7
Iteration: 31  Decomp: random  Elapsed: 175.9 secs  Obj: 467.8  Improved: -0.0%  IWoI: 0
Iteration: 32  Decomp: guided  Elapsed: 200.9 secs  Obj: 467.3  Improved: 0.1%  IWoI: 1
Advanced basis not built.
Iteration: 33  Decomp: guided  Elapsed: 225.9 secs  Obj: 467.3  Improved: -0.0%  IWoI: 0
Iteration: 34  Decomp: random  Elapsed: 229.8 secs  Obj: 467.3  Improved: 0.0%  IWoI: 1
Iteration: 35  Decomp: guided  Elapsed: 236.2 secs  Obj: 467.3  Improved: 0.0%  IWoI: 2
Iteration: 36  Decomp: guided  Elapsed: 261.2 secs  Obj: 467.3  Improved: 0.0%  IWoI: 3
Iteration: 37  Decomp: random  Elapsed: 262.0 secs  Obj: 467.3  Improved: 0.0%  IWoI: 4
Iteration: 38  Decomp: random  Elapsed: 265.5 secs  Obj: 467.3  Improved: 0.0%  IWoI: 5
Iteration: 39  Decomp: random  Elapsed: 265.5 secs  Obj: 467.3  Improved: 0.0%  IWoI: 6
Advanced basis not built.
Iteration: 40  Decomp: guided  Elapsed: 290.5 secs  Obj: 467.3  Improved: 0.0%  IWoI: 7
Iteration: 41  Decomp: random  Elapsed: 296.8 secs  Obj: 467.3  Improved: 0.0%  IWoI: 8
Iteration: 42  Decomp: guided  Elapsed: 299.0 secs  Obj: 467.3  Improved: 0.0%  IWoI: 9
Iteration: 43  Decomp: guided  Elapsed: 310.3 secs  Obj: 467.3  Improved: 0.0%  IWoI: 10
Iteration: 44  Decomp: guided  Elapsed: 321.6 secs  Obj: 467.3  Improved: 0.0%  IWoI: 11

Best solution found: 467.3

467.3
```
