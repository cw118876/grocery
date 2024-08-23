cmake_minimum_required(VERSION 3.11)
project(function)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DCMAKE_VERBOSE_MAKEFILE=ON)

include_directories(include)

include(cmake/functionCompileFlags.cmake)
include(cmake/findAllTarget.cmake)

add_subdirectory(src)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_subdirectory(test)
endif()

add_custom_target(
    copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Update compile_commands.json"
)

get_all_targets(all_targets)

# Loop over all targets and add them as dependencies to AllTargetsDepends
foreach(target ${all_targets})
    if(NOT target STREQUAL "copy_compile_commands") # Exclude the custom target itself
        add_dependencies(copy_compile_commands ${target})
    endif()
endforeach()