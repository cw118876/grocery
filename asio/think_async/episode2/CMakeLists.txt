
function(build_target fileName)
  set(target episode2_${fileName})
  add_executable(${target}
    ${fileName}.cpp
  )
  target_compile_features(${target}
  PRIVATE cxx_std_20
  )

  target_compile_definitions(${target}
  PUBLIC ASIO_ENABLE_HANDLER_TRACKING
  )

  target_link_libraries(${target}
    ${CONAN_LIBS}
  )

  install(
    TARGETS ${target}
    DESTINATION bin/episode2/
  )
endfunction()


build_target(step_0)
build_target(step_1)
build_target(step_2)
build_target(step_3)
build_target(step_4)
build_target(step_5)
build_target(step_6)
build_target(step_7)
build_target(step_8)