#include "fifo3.hpp"
#include "benchmark/bench.hpp"

int main(int argc, const char* argv[]) {
   bench<Fifo3>("Fifo3", argc, argv);
   return 0;
}