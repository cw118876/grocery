
add_executable(step_0
  step_0.cpp
)


target_compile_features(step_0
  PRIVATE cxx_std_20
)


target_compile_definitions(step_0
  PUBLIC ASIO_ENABLE_HANDLER_TRACKING
)


target_link_libraries(step_0
  ${CONAN_LIBS}
)

install(
    TARGETS step_0
    DESTINATION bin
)