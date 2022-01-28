cmake_minimum_required(VERSION 3.20)

# set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_C_STANDARD 11)
add_library(utils STATIC
        list.c headers/list.h
        str.c headers/str.h
        pair.c headers/pair.h
        map.c headers/map.h
        queue.c headers/queue.h
        memory.c headers/memory.h)

set(CC_WARN_FLAGS -Werror -Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter
        -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition
        -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wno-unknown-pragmas)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CC_WARN_FLAGS -Wjump-misses-init -Wlogical-op)
endif ()

target_compile_options(utils PRIVATE ${CC_WARN_FLAGS})

SET(DEBUG_MEMORY FALSE CACHE BOOL "Print memory access messages")
SET(CPOCL_MEMORY_DEBUG FALSE CACHE BOOL "Enable memory debug and stats")
SET(CPOCL_SHORT_NAMES FALSE CACHE BOOL "Enable short names")

if (DEBUG_MEMORY)
    if (APPLE)
        # use with `export MallocNanoZone=0`
        target_compile_options(tc PRIVATE -fsanitize=address)
        target_link_options(tc PRIVATE -fsanitize=address)
    endif ()
endif ()

if (CPOCL_SHORT_NAMES)
    add_compile_definitions(utils CPOCL_SHORT_NAMES)
endif ()
if (CPOCL_MEMORY_DEBUG)
    add_compile_definitions(utils CPOCL_MEMORY_DEBUG)
endif ()

add_executable(queue_test queue_test.c queue.c)
target_compile_features(queue_test PRIVATE c_std_99)
target_link_libraries(queue_test PRIVATE cmocka-static)

add_executable(list_test list_test.c list.c)
target_compile_features(list_test PRIVATE c_std_99)
target_link_libraries(list_test PRIVATE cmocka-static)

add_executable(map_test map_test.c map.c pair.c)
target_compile_features(map_test PRIVATE c_std_99)
target_link_libraries(map_test PRIVATE cmocka-static)

add_test(NAME queue_test COMMAND queue_test)
add_test(NAME list_test COMMAND list_test)
add_test(NAME map_test COMMAND map_test)