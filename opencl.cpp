//
// OpenCL benchmark for running a STREAM-like benchmark on GPUs to determine bandwidth vs. workgroup configuration
//
// Author: Karl Rupp,  me@karlrupp.net
// License: MIT/X11 license, see file LICENSE.txt
//

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "opencl-errcheck.hpp"
#include "benchmark-utils.hpp"


// Kernel for the benchmark
static const char * benchmark_program =
"__kernel void sum_buffer(\n"
"          __global const double * x,\n"
"          __global       double * sum,\n"
"          unsigned int size) \n"
"{ \n"
"  double s = 0;"
"  for (unsigned int i = 0; i < size; ++i)\n"
"    s += x[i];\n"
"  *sum = s;"
"};\n";

int main(int argc, char **argv)
{
  typedef double       NumericT;

  /////////////////////////// Part 1: Initialize OpenCL ///////////////////////////////////

  //
  // Query platform:
  //
  cl_uint num_platforms;
  cl_platform_id platform_ids[42];   //no more than 42 platforms supported...
  cl_int err = clGetPlatformIDs(42, platform_ids, &num_platforms); ERR_CHECK(err);

  std::cout << "# Platforms found: " << num_platforms << std::endl;
  for (cl_uint i=0; i<num_platforms; ++i)
  {
    char buffer[1024];
    err = clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VENDOR, 1024 * sizeof(char), buffer, NULL); ERR_CHECK(err);

    std::stringstream ss;
    ss << "# (" << i << ") " << buffer << ": ";

    err = clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VERSION, 1024 * sizeof(char), buffer, NULL); ERR_CHECK(err);

    ss << buffer;

    std::cout << ss.str() << std::endl;
  }

  std::size_t platform_index = 0;
  if (num_platforms > 1)
  {
    std::cout << "# Enter platform index to use: ";
    std::cin >> platform_index;
    platform_index = std::min<std::size_t>(platform_index, num_platforms - 1);
    std::cout << "#" << std::endl;
  }

  //
  // Query devices:
  //
  cl_device_id device_ids[42];
  cl_uint num_devices;
  err = clGetDeviceIDs(platform_ids[platform_index], CL_DEVICE_TYPE_ALL, 42, device_ids, &num_devices); ERR_CHECK(err);
  std::cout << "# Devices found: " << num_devices << std::endl;
  for (cl_uint i=0; i<num_devices; ++i)
  {
    char buffer[1024];
    err = clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, sizeof(char)*1024, &buffer, NULL); ERR_CHECK(err);

    std::cout << "# (" << i << ") " << buffer << std::endl;
  }

  std::size_t device_index = 0;
  if (num_devices > 1)
  {
    std::cout << "# Enter index of device to use: ";
    std::cin >> device_index;
    device_index = std::min<std::size_t>(device_index, num_devices - 1);
    std::cout << "#" << std::endl;
  }

  // now set up a context containing the selected device:
  cl_context my_context = clCreateContext(0, 1, &(device_ids[device_index]), NULL, NULL, &err); ERR_CHECK(err);

  // create a command queue for the device:
  cl_command_queue queue = clCreateCommandQueue(my_context, device_ids[device_index], 0, &err); ERR_CHECK(err);


  /////////////////////////// Part 2: Run benchmarks ///////////////////////////////////

  size_t source_len = std::string(benchmark_program).size();
  cl_program prog = clCreateProgramWithSource(my_context, 1, &benchmark_program, &(source_len), &err); ERR_CHECK(err);
  err = clBuildProgram(prog, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    char buffer[8192];
    cl_build_status status;
    std::cout << "Build Scalar: Err = " << err;
    err = clGetProgramBuildInfo(prog, device_ids[device_index], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL); ERR_CHECK(err);
    err = clGetProgramBuildInfo(prog, device_ids[device_index], CL_PROGRAM_BUILD_LOG,    sizeof(char)*8192, &buffer, NULL); ERR_CHECK(err);
    std::cout << " Status = " << status << std::endl;
    std::cout << "Log: " << buffer << std::endl;
    std::cout << "Sources: " << benchmark_program << std::endl;
  }
  cl_kernel kernel;
  cl_uint   num_kernels_ret = 0;
  err = clCreateKernelsInProgram(prog, 1, &kernel, &num_kernels_ret); ERR_CHECK(err);

  //
  // Benchmark
  //
  std::cout << "# N         Time          GB/sec" << std::endl;
  for (std::size_t N = 42; N < 1e6; N = (3*N) / 2)
  {
    std::vector<double> data_host(N);
    for (std::size_t i=0; i<N; ++i)
      data_host[i] = i;

    cl_uint buffer_size = data_host.size();

    cl_mem buf     = clCreateBuffer(my_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(double) * N, &(data_host[0]), &err); ERR_CHECK(err);
    cl_mem buf_sum = clCreateBuffer(my_context, CL_MEM_READ_WRITE,                        sizeof(double),     NULL,            &err); ERR_CHECK(err);

    Timer timer;
    std::vector<double> timings(10);
    for (std::size_t i=0; i<timings.size(); ++i)
    {
      double sum = 0;
      timer.start();
      for (std::size_t j=0; j<timings.size(); ++j)
      {
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf); ERR_CHECK(err);
        err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_sum); ERR_CHECK(err);
        err = clSetKernelArg(kernel, 2, sizeof(cl_uint), &buffer_size); ERR_CHECK(err);
        err = clEnqueueTask(queue, kernel, 0, NULL, NULL); ERR_CHECK(err);
        err = clEnqueueReadBuffer(queue, buf_sum, CL_TRUE, 0, 0, &sum, 0, NULL, NULL); ERR_CHECK(err);

        if (sum < 0)
          std::cerr << "Error: Computed sum negative!" << std::endl;
      }
      timings[i] = timer.get() / timings.size();
    }
    std::sort(timings.begin(), timings.end());
    double median_time = timings[timings.size()/2];
    std::cout << data_host.size() << "     " << median_time << "        " << data_host.size() * sizeof(double) / median_time / 1e9 << std::endl;

    err = clReleaseMemObject(buf);     ERR_CHECK(err);
    err = clReleaseMemObject(buf_sum); ERR_CHECK(err);
  }

  //
  // Part 3: Cleanup
  //
  err = clReleaseProgram(prog); ERR_CHECK(err);
  err = clReleaseCommandQueue(queue); ERR_CHECK(err);
  err = clReleaseContext(my_context); ERR_CHECK(err);

  return EXIT_SUCCESS;
}

