cmake_minimum_required(VERSION 3.11)
project(type-erasure)

find_package(benchmark REQUIRED)

add_executable(test 
  smartptr.cpp
)

target_link_libraries(test 
 PRIVATE benchmark::benchmark
)


target_compile_features(test
 PRIVATE cxx_std_17
)