CXX=g++
CXXFLAGS=-I. -O3


.PHONY: all
all: funcptr lua opencl



funcptr: funcptr.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS)

lua: lua.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) -llua5.2
    #adjust OpenMP flag as needed

opencl: opencl.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) -lOpenCL

clean:
	rm funcptr lua opencl
