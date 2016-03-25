set terminal postscript enhanced color eps

set style data lines
set style line 1  linetype -1 linewidth 3 lc rgb "#990000"
set style line 2  linetype -1 linewidth 3 lc rgb "#009900"
set style line 3  linetype -1 linewidth 3 lc rgb "#000099"
set style increment user

set size 0.75,0.75
set border lw 2

set key bottom right Left reverse
set grid
set logscale x
set xlabel "Array Size (doubles)"
set format x "10^{%L}"
set format y "10^{%L}"

#######


# Plot time for latency
set output "lua-opencl-funcptr-time.eps"
set logscale y
set ylabel "Time (sec)"
set xrange [40:4e4]
set title "Latency Comparison of Lua, OpenCL, and C/C++ Function Pointers"
plot 'lua.txt'         using 1:2 with linesp ls 1 pt  5 ps 1.5 title "Lua 5.2", \
     'opencl.txt'      using 1:2 with linesp ls 2 pt  7 ps 1.5 title "Intel OpenCL 1.2, v5.0.0.43", \
     'funcptr.txt'     using 1:2 with linesp ls 3 pt  9 ps 1.5 title "Plain C/C++"


# Plot bandwidth
set key bottom right Left reverse invert
set output "lua-opencl-funcptr-bandwidth.eps"
set title "Bandwidth Comparison of Lua, OpenCL, and C/C++ Function Pointers"
#set nolog y
set ylabel "Bandwidth (GB/sec)"
set xrange [10:1e6]
plot 'lua.txt'        using 1:3 with linesp ls 1 pt  5 ps 1.5 title "Lua 5.2", \
     'opencl.txt'     using 1:3 with linesp ls 2 pt  7 ps 1.5 title "Intel OpenCL 1.2, v5.0.0.43", \
     'funcptr.txt'    using 1:3 with linesp ls 3 pt  9 ps 1.5 title "Plain C/C++"


