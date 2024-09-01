
set(target bookmark_service)

find_package(asio REQUIRED)
find_package(nlohmann_json REQUIRED)




add_executable(${target}
  main.cpp
)

target_compile_definitions(${target}
  PUBLIC ASIO_ENABLE_HANDLER_TRACKING
  )

target_link_libraries(${target}
  PUBLIC
  asio::asio
  nlohmann_json::nlohmann_json
)

install(
  TARGETS ${target}
  DESTINATION bin/
)