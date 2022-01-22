cmake_minimum_required(VERSION 3.20)

# set(CMAKE_VERBOSE_MAKEFILE on)

add_subdirectory(utils)
include_directories(utils/headers)

set(CMAKE_C_STANDARD 11)
add_executable(tc
        main.c
        lexer.c lexer.h
        parser.c parser.h
        analyser.c analyser.h
        symbol.c symbol.h
        symbol_table.c symbol_table.h
        ast.c ast.h
        utils/headers/functions.h
        utils/headers/list.h
        utils/headers/map.h
        utils/headers/queue.h
        utils/headers/str.h)
target_link_libraries(tc PRIVATE utils)
target_link_libraries(tc PRIVATE m)

SET(VM_DEBUG_MEMORY FALSE CACHE BOOL "Print memory access messages")

if (VM_DEBUG_MEMORY)
    if (APPLE)
        # use with `export MallocNanoZone=0`
        target_compile_options(tc PRIVATE -fsanitize=address)
        target_link_options(tc PRIVATE -fsanitize=address)
    endif()
endif()
