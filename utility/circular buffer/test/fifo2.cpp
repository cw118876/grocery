#include "fifo2.hpp"
#include "benchmark/bench.hpp"

int main(int argc, const char* argv[]) {
   bench<Fifo2>("Fifo2", argc, argv);
   return 0;
}