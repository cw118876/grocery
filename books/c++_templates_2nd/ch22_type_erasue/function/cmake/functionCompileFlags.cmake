function(get_compile_common_flags compile_common_flags build_type)
    set(predefined_common_flags "-Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
-D_GLIBCXX_ASSERTIONS \
-fstack-clash-protection -fstack-protector-strong \
-fdiagnostics-color=always"
    )

    if(build_type MATCHES "Debug")
        # --coverage: enable coverage compile
        # -fprofile-arcs: generate .gcno file which contains information to reconstruct basic blocks graph and
        # assign source line number of blocks
        # -fprofile-arcs: generate .gcda count file such that a separate .gcda file is created for each objec file.
        set(${compile_common_flags} "-g -O0 -fno-inline --coverage -fprofile-arcs -ftest-coverage ${predefined_common_flags}" PARENT_SCOPE)
    else()
        set(${compile_common_flags} "-g -O2 ${predefined_common_flags}" PARENT_SCOPE)
    endif()
endfunction()

get_compile_common_flags(ordinary_common_flags ${CMAKE_BUILD_TYPE})

set(CMAKE_C_FLAGS ${ordinary_common_flags})
set(CMAKE_CXX_FLAGS ${ordinary_common_flags})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)