
set(target functional)
set(target_shared functional_shared)

add_library(${target}
  function.cpp
)

add_library(${target_shared}
  SHARED function.cpp
)

set_target_properties(${target_shared} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${target_shared} PROPERTIES OUTPUT_NAME ${target})

install(
  TARGETS ${target}
  DESTINATION lib/
)


install(
  TARGETS ${target_shared}
  DESTINATION lib/
)