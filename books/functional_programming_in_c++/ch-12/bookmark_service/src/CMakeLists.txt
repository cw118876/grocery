
set(target bookmark_service)

add_executable(${target}
  bookmark_service.cpp
)


install(
  TARGETS ${target}
  DESTINATION bin/
)