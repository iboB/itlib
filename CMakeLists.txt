cmake_minimum_required(VERSION 3.2)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # dev_mode is used below to make life simpler for developers
    # it enables some configurations and the defaults for building tests and examples
    # which typically wouldn't be built if itlib is a subdirectory of another project
    set(dev_mode ON)
    if(POLICY CMP0092)
        cmake_policy(SET CMP0092 NEW) # don't add /W3 to cached compiler flags
    endif()
else()
    set(dev_mode OFF)
endif()

project(itlib)

if(dev_mode)
    include(./dev.cmake)
endif()

option(ITLIB_BUILD_TESTS "itlib: build tests" ${dev_mode})
mark_as_advanced(ITLIB_BUILD_TESTS)

add_library(itlib INTERFACE)
add_library(itlib::itlib ALIAS itlib)
target_include_directories(itlib INTERFACE include)

if(ITLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
