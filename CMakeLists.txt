# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.24)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # dev_mode is used below to make life simpler for developers
    # it enables some configurations and the defaults for building tests and examples
    # which typically wouldn't be built if itlib is a subdirectory of another project
    set(dev_mode ON)
else()
    set(dev_mode OFF)
endif()

project(itlib
    LANGUAGES CXX
    VERSION 1.11.9
)

if(dev_mode)
    include(./dev.cmake)
endif()

option(ITLIB_BUILD_TESTS "itlib: build tests" ${dev_mode})
mark_as_advanced(ITLIB_BUILD_TESTS)

add_subdirectory(include)

if(ITLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

######################
# install
set(install_destination share/cmake/itlib)

install(EXPORT itlib-targets
    NAMESPACE itlib::
    DESTINATION ${install_destination}
)
configure_file(
    itlib-config-version.in.cmake
    itlib-config-version.cmake
    @ONLY
)
configure_file(
    itlib-config.in.cmake
    itlib-config.cmake
    @ONLY
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/itlib-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/itlib-config-version.cmake
    DESTINATION ${install_destination}
)
