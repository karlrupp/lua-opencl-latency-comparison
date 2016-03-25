#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <lua5.2/lua.hpp>


#include "benchmark-utils.hpp"

double run_bench(std::size_t N, lua_State *L)
{
    // first the function to work with
    lua_getglobal(L, "foo");

    // push copy of table/array stored at the bottom of the stack
    lua_pushvalue(L, 1);
    lua_pushnumber(L, N);

    /* Ask Lua to run our little script */
    lua_call(L, 2, 1);

    /* Get the returned value at the top of the stack (index -1) */
    double sum = lua_tonumber(L, -1);
    lua_pop(L, 1);

    return sum;
}


int main(void)
{
  //create a Lua state
  lua_State *L = luaL_newstate();

  luaL_openlibs(L); /* Load Lua libraries */

  /* Load the file containing the script we are going to run */
  int status = luaL_dofile(L, "script.lua");
  if (status) {
    /* If something went wrong, error message is at the top of */
    /* the stack */
    fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
    exit(1);
  }

  std::cout << "# N         Time          GB/sec" << std::endl;
  for (std::size_t N=42; N<1e6; N = (3*N) / 2)
  {
    lua_createtable(L, N, 0);    /* We will pass a table */

    for (std::size_t i = 1; i <= N; i++) {
        lua_pushnumber(L, i);   /* Push the table index */
        lua_pushnumber(L, i);   /* Push the cell value */
        lua_rawset(L, -3);      /* Stores the pair in the table */
    }

    Timer timer;
    double sum = 0;
    std::vector<double> timings(10);
    for (std::size_t i=0; i<timings.size(); ++i)
    {
      timer.start();
      for (std::size_t j=0; j<timings.size(); ++j)
      {
        sum = run_bench(N, L);

        if (sum < 0)
          std::cerr << "ERROR: Negative sum of nonnegative values obtained!" << std::endl;
      }
      timings[i] = timer.get() / timings.size();
    }
    std::sort(timings.begin(), timings.end());
    double median_time = timings[timings.size()/2];
    std::cout << N << "      "
              << median_time      << "      "
              << N * sizeof(double) / median_time / 1e9 << std::endl;

    lua_pop(L, 1);
  }

  lua_close(L); //close Lua state

  return EXIT_SUCCESS;
}

