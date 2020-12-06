# Advent of Code 2020
https://adventofcode.com/

My entries to Advent of Code 2020. Using the onut framework.

## Generate VS solution
```
git submodule update --init --recursive
mkdir build
cd build
cmake ..
cmake ..
```
Then open solution, or `make` on OSX/Linux. `cmake ..` is called twice because there is a setting problem in onut, where second time it gets picked up properly.
