# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

include(./get_cpm.cmake)

CPMAddPackage(gh:iboB/icm@1.5.1)
CPMAddPackage(gh:iboB/doctest-util@0.1.2)

include(${icm_SOURCE_DIR}/icm_build_failure_testing.cmake)

macro(define_doctest_lib std)
    add_library(doctest-${std} STATIC doctest_main.cpp)
    set_target_properties(doctest-${std} PROPERTIES CXX_STANDARD ${std})
    target_link_libraries(doctest-${std} PUBLIC doctest::headers)
endmacro()

define_doctest_lib(11)
define_doctest_lib(14)
define_doctest_lib(17)
define_doctest_lib(20)

macro(add_test_for_std test std)
    set(fname t-${test}-${std}.cpp)
    set(targetname itlib-test-${test}-${std})
    add_executable(${targetname} ${fname})
    set_target_properties(${targetname} PROPERTIES
        CXX_STANDARD ${std}
    )
    target_link_libraries(${targetname} doctest-${std} doctest::util itlib::itlib ${CMAKE_THREAD_LIBS_INIT})

    set(testname itlib-${test}-${std} )
    add_test(NAME ${testname} COMMAND ${targetname})

    set(labels ${ARGN})
    if(labels)
        set_tests_properties(${testname} PROPERTIES LABELS "${labels}")
    endif()
endmacro()

macro(add_test_if_exist test std)
    set(fname t-${test}-${std}.cpp)

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${fname})
        add_test_for_std(${test} ${std} ${ARGN})
    endif()
endmacro()

macro(add_itlib_test test)
    add_test_if_exist(${test} 11 ${ARGN})
    add_test_if_exist(${test} 14 ${ARGN})
    add_test_if_exist(${test} 17 ${ARGN})
    add_test_if_exist(${test} 20 ${ARGN})
endmacro()

add_itlib_test(itlib)

add_itlib_test(any)
add_itlib_test(dynamic_bitset)
add_itlib_test(expected)
add_itlib_test(flat_map)
add_itlib_test(flat_set)
add_itlib_test(generator)
add_itlib_test(make_ptr)
add_itlib_test(memory_view)
add_itlib_test(mem_streambuf)
add_itlib_test(pmr_allocator)
add_itlib_test(pod_vector)
add_itlib_test(poly_span)
add_itlib_test(qalgorithm)
add_itlib_test(rstream)
add_itlib_test(sentry)
add_itlib_test(shared_from)
add_itlib_test(static_vector)
add_itlib_test(strutil)
add_itlib_test(small_vector)
add_itlib_test(span)
add_itlib_test(stride_span)
add_itlib_test(throw_ex)
add_itlib_test(time_t)
add_itlib_test(transparent_umap)
add_itlib_test(type_traits)
add_itlib_test(ufunction)
add_itlib_test(utility)

find_package(Threads)
if (Threads_FOUND)
    add_itlib_test(atomic tsan)
    add_itlib_test(atomic_shared_ptr_storage tsan)
    add_itlib_test(data_mutex tsan)
    add_itlib_test(mutex tsan)
endif()

icm_add_multiple_build_failure_tests(
    SOURCES
        bft-small_vector-revert-cap-mismatch.cpp
        bft-pod_vector-pod.cpp
        bft-utility-force_move.cpp
    LIBRARIES
        itlib
    PREFIX
        itlib
    PROPERTIES
        CXX_STANDARD 11
)
