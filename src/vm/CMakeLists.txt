cmake_minimum_required(VERSION 3.20)
project(thm C)

include(cmake/FetchCCANJson.cmake)
include(cmake/FetchMiniFB.cmake)

# set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_C_STANDARD 11)
add_executable(thm
        main.c
        opts.c opts.h
        vmarch.c vmarch.h
        vm.c vm.h
        bus.c bus.h
        cpu.c cpu.h
        ops.c ops.h
        pic.c pic.h
        memory.c memory.h
        video.c video.h
        keyboard.c keyboard.h
        time.c time.h
        pit.c pit.h
        debugger_lexer.c debugger_lexer.h
        debugger.c debugger.h
        json.c json.h ${CCAN_JSON})
target_link_libraries(thm PRIVATE minifb)

if(UNIX)
    if (NOT APPLE)
        target_link_libraries(thm PRIVATE pthread)
    endif()
endif()

SET(VM_DEBUG_MEMORY FALSE CACHE BOOL "Print memory access messages")

if (VM_DEBUG_MEMORY)
    if (APPLE)
        # use with `export MallocNanoZone=0`
        target_compile_options(thm PRIVATE -fsanitize=address)
        target_link_options(thm PRIVATE -fsanitize=address)
        add_compile_definitions(WITH_MEMORY_LOG_MESSAGE)
    endif()
endif()

# tests
include(cmake/FetchCMocka.cmake)

add_executable(pic_test pic_test.c pic.c memory.c) # todo get rid of memory.c here...
target_compile_features(pic_test PRIVATE c_std_99)
target_link_libraries(pic_test PRIVATE cmocka-static)

add_executable(memory_test memory_test.c memory.c)
target_compile_features(memory_test PRIVATE c_std_99)
target_link_libraries(memory_test PRIVATE cmocka-static)

add_executable(bus_test bus_test.c bus.c memory.c) # todo get rid of memory.c here...
target_compile_features(bus_test PRIVATE c_std_99)
target_link_libraries(bus_test PRIVATE cmocka-static)

add_executable(cpu_test cpu_test.c cpu.c)
target_compile_features(cpu_test PRIVATE c_std_99)
target_link_libraries(cpu_test PRIVATE cmocka-static)

enable_testing()
add_test(NAME memory_test COMMAND memory_test)
add_test(NAME pic_test COMMAND pic_test)
add_test(NAME bus_test COMMAND bus_test)
add_test(NAME cpu_test COMMAND bus_test)
