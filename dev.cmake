set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ITLIB_TSAN "itlib: build with thread sanitizer on" OFF)
option(ITLIB_ASAN "itlib: build with address sanitizer on" OFF)
option(ITLIB_CLANG_TIDY "itlib: use clang tidy" OFF)

set(ITLIB_SAN_FLAGS "")
if(MSVC)    
    add_compile_options(
        /W4
        -D_CRT_SECURE_NO_WARNINGS /Zc:__cplusplus /permissive-
        /volatile:iso /Zc:throwingNew /utf-8 -DNOMINMAX=1
        /wd4251 /wd4275
    )
else()
    add_compile_options(-Wall -Wextra -Wno-type-limits)
    if(ITLIB_CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY clang-tidy)
    endif()
endif()

if(ITLIB_TSAN)
    if(NOT MSVC)
        set(ITLIB_SAN_FLAGS -fsanitize=thread -g)
    endif()
elseif(ITLIB_ASAN)
    if(MSVC)
        set(ITLIB_SAN_FLAGS /fsanitize=address)
    elseif(APPLE)
        # apple clang doesn't support the leak sanitizer
        set(ITLIB_SAN_FLAGS -fsanitize=address,undefined -pthread -g)
    else()
        set(ITLIB_SAN_FLAGS -fsanitize=address,undefined,leak -pthread -g)
    endif()
endif()

add_compile_options(${ITLIB_SAN_FLAGS})
if(NOT MSVC)
    add_link_options(${ITLIB_SAN_FLAGS})
endif()
