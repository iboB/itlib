add_library(doctest-11 STATIC doctest_main.cpp)
set_target_properties(doctest-11 PROPERTIES CXX_STANDARD 11)
add_library(doctest-14 STATIC doctest_main.cpp)
set_target_properties(doctest-14 PROPERTIES CXX_STANDARD 14)
add_library(doctest-17 STATIC doctest_main.cpp)
set_target_properties(doctest-17 PROPERTIES CXX_STANDARD 17)

macro(add_test_for_std test std)
    set(fname t-${test}-${std}.cpp)
    set(tname itlib-test-${test}-${std})
    add_executable(${tname} ${fname})
    set_target_properties(${tname} PROPERTIES
        CXX_STANDARD ${std}
    )
    target_link_libraries(${tname} doctest-${std} itlib ${CMAKE_THREAD_LIBS_INIT})

    add_test(NAME itlib-${test}-${std} COMMAND $<TARGET_FILE:${tname}>)
endmacro()

macro(add_test_if_exist test std)
    set(fname t-${test}-${std}.cpp)

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${fname})
        add_test_for_std(${test} ${std})
    endif()
endmacro()

macro(add_itlib_test test)
    add_test_for_std(${test} 11)
    add_test_if_exist(${test} 14)
    add_test_if_exist(${test} 17)
endmacro()

add_itlib_test(flat_map)
add_itlib_test(make_ptr)
add_itlib_test(memory_view)
add_itlib_test(mem_streambuf)
add_itlib_test(sentry)
add_itlib_test(static_vector)
add_itlib_test(small_vector)
add_itlib_test(ufunction)
