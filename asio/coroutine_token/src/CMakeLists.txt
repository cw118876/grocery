

set(target coro_token)

add_executable(${target}
  main.cpp
)

target_compile_features(${target}
  PRIVATE cxx_std_20
)

target_compile_definitions(${target}
  PUBLIC ASIO_ENABLE_HANDLER_TRACKING
)

target_link_libraries(${target}
  PRIVATE asio::asio
)

install(
  TARGETS ${target}
  DESTINATION bin/
)