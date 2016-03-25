#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>


#include "benchmark-utils.hpp"

// worker function computing the sum of all entries in an array  'x' of size 'N'
double compute_sum(double *x, std::size_t N)
{
  double sum = 0;
  for (std::size_t i=0; i<N; ++i)
    sum += x[i];

  return sum;
}

// dummy to avoid compiler inlining:
double compute_sum2(double *, std::size_t) { return -1.0; }


int main(int argc, char **argv)
{
  // function pointer via which worker routine is called
  double (*worker)(double*, std::size_t) = compute_sum;

  std::cout << "# N         Time          GB/sec" << std::endl;
  for (std::size_t N=42; N<1e6; N = (3*N) / 2)
  {
    std::vector<double> x(N);
    for (std::size_t i=0; i<N; ++i)
      x[i] = i;

    if (argc < 0)             // make sure compiler doesn't inline function call
      worker = compute_sum2;

    double sum = 0;

    Timer timer;
    std::vector<double> timings(10);
    for (std::size_t i=0; i<timings.size(); ++i)
    {
      timer.start();
      for (std::size_t j=0; j<timings.size(); ++j)
      {
        sum = worker(&(x[0]), N);

        if (sum < 0)
          std::cerr << "Error: Computed sum negative!" << std::endl;
      }
      timings[i] = timer.get() / timings.size();
    }
    std::sort(timings.begin(), timings.end());
    double median_time = timings[timings.size()/2];
    std::cout << N << "      "
              << median_time      << "      "
              << N * sizeof(double) / median_time / 1e9 << std::endl;
  }

  return EXIT_SUCCESS;
}

