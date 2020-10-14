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
