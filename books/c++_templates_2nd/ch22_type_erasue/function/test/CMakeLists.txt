set(project_test_sources
    test_main.cpp
    src/count_new.cpp
    unit_test/bad_call/bad_fuction_call_ctor.cpp
    unit_test/bad_call/bad_function_call.cpp
    unit_test/func/swap.cpp
    unit_test/func/operator_bool.cpp
    unit_test/func/common/copy_assign.cpp
    unit_test/func/common/copy_move.cpp
    unit_test/func/common/default.cpp
    unit_test/func/common/F_assign.cpp
    unit_test/func/common/F_incomplete.cpp
    unit_test/func/common/F_nullptr.cpp
    unit_test/func/common/F.cpp
    unit_test/func/common/nullptr_t_assign.cpp
    unit_test/func/common/nullptr_t.cpp
    unit_test/func/common/swap.cpp
    unit_test/func/invoke.cpp
    unit_test/func/addressof.cpp
)

add_executable(test.function ${project_test_sources})

target_include_directories(test.function PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(test.function
    PUBLIC
    GTest::gtest
    GTest::gmock
    functional
)
