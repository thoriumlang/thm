cmake_minimum_required(VERSION 3.20)

# set(CMAKE_VERBOSE_MAKEFILE on)

add_subdirectory(utils)
include_directories(utils/headers)

set(CMAKE_C_STANDARD 11)
add_executable(tc
        main.c
        lexer.c lexer.h
        parser.c parser.h
        ast.c ast.h
        utils/headers/list.h
        utils/headers/queue.h
        utils/headers/str.h)
target_link_libraries(tc PRIVATE utils)


SET(VM_DEBUG_MEMORY FALSE CACHE BOOL "Print memory access messages")

if (VM_DEBUG_MEMORY)
    if (APPLE)
        # use with `export MallocNanoZone=0`
        target_compile_options(tc PRIVATE -fsanitize=address)
        target_link_options(tc PRIVATE -fsanitize=address)
    endif()
endif()
