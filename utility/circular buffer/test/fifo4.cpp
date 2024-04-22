#include "fifo4.hpp"
#include "benchmark/bench.hpp"

int main(int argc, const char* argv[]) {
   bench<Fifo4>("Fifo4", argc, argv);
   return 0;
}