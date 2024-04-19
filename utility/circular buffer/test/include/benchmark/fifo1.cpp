#include "fifo1.hpp"
#include "benchmark/bench.hpp"

int main(int argc, const char* argv[]) {
   bench<Fifo1>("Fifo1", argc, argv);
   return 0;
}