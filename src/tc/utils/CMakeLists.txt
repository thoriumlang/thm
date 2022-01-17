cmake_minimum_required(VERSION 3.20)

# set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_C_STANDARD 11)
add_library(utils STATIC
        list.c headers/list.h
        str.c headers/str.h
        queue.c headers/queue.h)

SET(VM_DEBUG_MEMORY FALSE CACHE BOOL "Print memory access messages")

if (VM_DEBUG_MEMORY)
    if (APPLE)
        # use with `export MallocNanoZone=0`
        target_compile_options(tc PRIVATE -fsanitize=address)
        target_link_options(tc PRIVATE -fsanitize=address)
    endif()
endif()

add_executable(queue_test queue_test.c queue.c)
target_compile_features(queue_test PRIVATE c_std_99)
target_link_libraries(queue_test PRIVATE cmocka-static)

add_executable(list_test list_test.c list.c)
target_compile_features(list_test PRIVATE c_std_99)
target_link_libraries(list_test PRIVATE cmocka-static)

add_test(NAME queue_test COMMAND queue_test)
add_test(NAME list_test COMMAND list_test)