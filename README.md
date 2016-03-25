
# Benchmark Code for comparing Lua, OpenCL, and native C/C++ functions

This repository contains the benchmark code supplementing my [blog post on a latency comparison of Lua, OpenCL, and plain C/C++](https://www.karlrupp.net/2016/03/lua-opencl-latency-comparison/).


## Build

On *nix-like systems it is usually enough to call `make`:

    $> make

If you run into issues, have a look at `makefile` and adjust compilers, etc.

Windows users should create a new project file in their favorite IDE and compile each of the three main applications `funcptr`, `lua`, or `opencl`.

## Run

Run the executables via

    $> ./funcptr
    $> ./lua
    $> ./opencl

Each of the three executables prints a table of execution times and bandwidths for different array sizes.

## License

The code is provided under a permissive MIT/X11-style license.
See file LICENSE.txt for details.

