# standard
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# misc config
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # use solution folders

if(MSVC)
    add_compile_options(
        -W4
        -D_CRT_SECURE_NO_WARNINGS -Zc:__cplusplus -permissive-
        -volatile:iso -Zc:throwingNew -Zc:templateScope -utf-8 -DNOMINMAX=1
        -wd4251 -wd4275
    )
else()
    add_compile_options(-Wall -Wextra -Wno-type-limits)
    if(ITLIB_CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY clang-tidy)
    endif()
endif()


# sanitizers

option(SAN_THREAD "itlib: sanitize thread" OFF)
option(SAN_ADDR "itlib: sanitize address" OFF)
option(SAN_UB "itlib: sanitize undefined behavior" OFF)
option(SAN_LEAK "itlib: sanitize leaks" OFF)

if(MSVC)
    if(SAN_ADDR)
        add_compile_options(-fsanitize=address)
    endif()
    if(SAN_THREAD OR SAN_UB OR SAN_LEAK)
        message(WARNING "Unsupported sanitizers requested for msvc. Ignored")
    endif()
else()
    if(SAN_THREAD)
        set(itlibSanFlags -fsanitize=thread -g)
        if(SAN_ADDR OR SAN_UB OR SAN_LEAK)
            message(WARNING "Incompatible sanitizer combination requested. Only 'SAN_THREAD' will be respected")
        endif()
    else()
        if(SAN_ADDR)
            list(APPEND itlibSanFlags -fsanitize=address -pthread)
        endif()
        if(SAN_UB)
            list(APPEND itlibSanFlags -fsanitize=undefined)
        endif()
        if(SAN_LEAK)
            if(APPLE)
                message(WARNING "Unsupported leak sanitizer requested for Apple. Ignored")
            else()
                list(APPEND itlibSanFlags -fsanitize=leak)
            endif()
        endif()
    endif()
    if(itlibSanFlags)
        add_compile_options(${itlibSanFlags})
        add_link_options(${itlibSanFlags})
    endif()
endif()
