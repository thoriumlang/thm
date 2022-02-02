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
        utils/headers/str.h
        utils/headers/macros.h)
target_link_libraries(tc PRIVATE utils)
target_link_libraries(tc PRIVATE m)

if (CPOCL_SHORT_NAMES)
    add_compile_definitions(tc CPOCL_SHORT_NAMES)
endif ()
if (CPOCL_MEMORY_DEBUG)
    add_compile_definitions(tc CPOCL_MEMORY_DEBUG)
endif ()

#set(CC_WARN_FLAGS -Werror -Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter
#        -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition
#        -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wno-unknown-pragmas)
#
#if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#    list(APPEND CC_WARN_FLAGS -Wjump-misses-init -Wlogical-op)
#endif ()
#
#target_compile_options(tc PRIVATE ${CC_WARN_FLAGS})


SET(VM_DEBUG_MEMORY FALSE CACHE BOOL "Print memory access messages")

if (VM_DEBUG_MEMORY)
    if (APPLE)
        # use with `export MallocNanoZone=0`
        target_compile_options(tc PRIVATE -fsanitize=address)
        target_link_options(tc PRIVATE -fsanitize=address)
    endif ()
endif ()
