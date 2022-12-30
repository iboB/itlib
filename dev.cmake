set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ITLIB_TSAN "itlib: build with thread sanitizer on" OFF)
option(ITLIB_ASAN "itlib: build with address sanitizer on" OFF)
option(ITLIB_CLANG_TIDY "itlib: use clang tidy" OFF)

set(ITLIB_SAN_FLAGS "")
if(MSVC)
    set(ITLIB_WARNING_FLAGS "-D_CRT_SECURE_NO_WARNINGS /Zc:__cplusplus /permissive-\
        /w34100 /w34189 /w34701 /w34702 /w34703 /w34706 /w34714 /w34913\
        /wd4251 /wd4275"
    )
else()
    set(ITLIB_WARNING_FLAGS "-Wall -Wextra -Wno-type-limits")
    if(ITLIB_CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY clang-tidy)
    endif()
endif()

if(ITLIB_TSAN)
    if(NOT MSVC)
        set(ITLIB_SAN_FLAGS "-fsanitize=thread -g")
    endif()
elseif(ITLIB_ASAN)
    if(MSVC)
        set(ITLIB_SAN_FLAGS "/fsanitize=address")
    elseif(APPLE)
        # apple clang doesn't support the leak sanitizer
        set(ITLIB_SAN_FLAGS "-fsanitize=address,undefined -pthread -g")
    else()
        set(ITLIB_SAN_FLAGS "-fsanitize=address,undefined,leak -pthread -g")
    endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ITLIB_WARNING_FLAGS} ${ITLIB_SAN_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ITLIB_WARNING_FLAGS} ${ITLIB_SAN_FLAGS}")
if(NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ITLIB_SAN_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ITLIB_SAN_FLAGS}")
endif()
