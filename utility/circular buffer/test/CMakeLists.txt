

add_executable(fifo1
 fifo1.cpp
)

add_executable(fifo1.tsan
 fifo1.cpp
)


target_include_directories(fifo1
 PRIVATE ${CMAKE_SOURCE_DIR}/include
 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(fifo1
 PRIVATE cxx_std_20
)

# target_compile_options(fifo1
#  PRIVATE -O3
# )


target_include_directories(fifo1.tsan
 PRIVATE ${CMAKE_SOURCE_DIR}/include
 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(fifo1.tsan
 PRIVATE cxx_std_20
)

target_compile_options(fifo1.tsan

 PRIVATE -fsanitize=thread

)


target_link_libraries(fifo1.tsan
 PRIVATE pthread tsan
)