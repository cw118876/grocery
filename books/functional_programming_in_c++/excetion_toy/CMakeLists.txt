cmake_minimum_required(VERSION 3.11)
project(execution_toy)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DCMAKE_VERBOSE_MAKEFILE=ON)

include_directories(include)

add_executable(sync_test
   src/sync_test.cpp
)


install(
  TARGETS sync_test
  DESTINATION bin/
)