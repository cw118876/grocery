cmake_minimum_required(VERSION 3.11)
project(Think_Async)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ASIO REQUIRED)

add_definitions(-DCMAKE_VERBOSE_MAKEFILE=ON)
set(CMAKE_C_FLAGS "-O2 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough\
    -Werror=format-security \
    -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
    -D_GLIBCXX_ASSERTIONS \
    -fstack-clash-protection -fstack-protector-strong \
    -Wl,-z,nodlopen -Wl,-z,noexecstack \
    -Wl,-z,relro -Wl,-z,now -fdiagnostics-color=always"
)
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough\
    -Werror=format-security \
    -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
    -D_GLIBCXX_ASSERTIONS \
    -fstack-clash-protection -fstack-protector-strong \
    -Wl,-z,nodlopen -Wl,-z,noexecstack \
    -Wl,-z,relro -Wl,-z,now -Wno-reorder -fdiagnostics-color=always"
)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    message("Enter Debug build mode")
    set(CMAKE_C_FLAGS "-g ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-fno-inline -g ${CMAKE_CXX_FLAGS}")
endif()


add_subdirectory(src)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        /var/tmp/compile_commands.json
        
)