

cmake_minimum_required(VERSION 3.11)


add_executable(foo
  foo.cpp
)

target_compile_features(foo
  PRIVATE cxx_std_20
)

target_include_directories(foo
 PRIVATE ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(foo
 PRIVATE my_any
)