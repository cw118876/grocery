set(project_test_sources
    test_main.cpp
    unit_test/test_bookmark_service.cpp
)

add_executable(test.bookmark_service ${project_test_sources})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(test.bookmark_service
    PUBLIC
    GTest::gtest
    GTest::gmock
)
