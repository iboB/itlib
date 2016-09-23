cmake_minimum_required(VERSION 2.8)

project(chobo-shl-test)

enable_testing()

set(INC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${INC})

file(GLOB_RECURSE choboshl "${INC}/chobo/*.hpp")
source_group("chobo" FILES ${choboshl})
file(GLOB doctest ${CMAKE_CURRENT_SOURCE_DIR}/doctest/*.h)
source_group("third_party" FILES ${doctest})

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

add_executable(chobo-shl-test
    ${choboshl}
    ${doctest}
    test.cpp
)

add_test(chobo-shl-test chobo-shl-test)
