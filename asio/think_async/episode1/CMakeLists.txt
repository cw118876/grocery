
add_executable(test 
  main.cpp
)

target_link_libraries(test
  ${CONAN_LIBS}
)

install(
    TARGETS test
    DESTINATION bin
)