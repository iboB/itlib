add_library(doctest STATIC
    doctest.hpp
    doctest_main.cpp
)

macro(add_itlib_test test)
    set(tname itlib-test-${test})
    add_executable(${tname} ${ARGN})
    target_link_libraries(${tname} doctest itlib ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${tname} PROPERTIES FOLDER test)
    add_test(NAME itlib-${test} COMMAND $<TARGET_FILE:${tname}>)
endmacro()

add_itlib_test(flat_map t-flat_map.cpp)
add_itlib_test(make_ptr t-make_ptr.cpp)
add_itlib_test(memory_view t-memory_view.cpp)
add_itlib_test(static_vector t-static_vector.cpp)
add_itlib_test(small_vector t-small_vector.cpp)
add_itlib_test(ufunction t-ufunction.cpp)
