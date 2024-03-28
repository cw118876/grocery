

cmake_minimum_required(VERSION 3.11)


add_library(my_any
 any.cpp
)



target_compile_features(my_any
  PRIVATE cxx_std_20
)
target_include_directories( my_any
 PRIVATE ${CMAKE_SOURCE_DIR}/include
)