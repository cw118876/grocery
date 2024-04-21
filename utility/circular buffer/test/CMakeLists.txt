function(add_fifo fifo) 
    add_executable(${fifo}
    ${fifo}.cpp
    )

    target_include_directories(${fifo}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_features(${fifo}
    PRIVATE cxx_std_20
    )
    add_executable(${fifo}.tsan
    ${fifo}.cpp
    )
    target_include_directories(${fifo}.tsan
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_features(${fifo}.tsan
    PRIVATE cxx_std_20
    )

    target_compile_options(${fifo}.tsan
    PRIVATE -fsanitize=thread
    )

    target_link_libraries(${fifo}.tsan
    PRIVATE pthread tsan
    )
endfunction()

add_fifo(fifo1)
add_fifo(fifo2)
add_fifo(fifo3)