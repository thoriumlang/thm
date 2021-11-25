cmake_minimum_required(VERSION 3.20)
project(c_vm C)

include(cmake/FetchCCANJson.cmake)
include(cmake/FetchMiniFB.cmake)

# set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_C_STANDARD 11)
add_executable(c_vm
        main.c
        opts.c opts.h
        vmarch.c vmarch.h
        bus.c bus.h
        cpu.c cpu.h
        ops.c ops.h
        memory.c memory.h
        video.c video.h
        json.c json.h ${CCAN_JSON})
target_link_libraries(c_vm PRIVATE minifb)

if(UNIX)
    if (NOT APPLE)
        target_link_libraries(c_vm PRIVATE pthread)
    endif()
endif()

# tests
include(cmake/FetchCMocka.cmake)

add_executable(memory_test memory_test.c memory.c)
target_compile_features(memory_test PRIVATE c_std_99)
target_link_libraries(memory_test PRIVATE cmocka-static)

add_executable(bus_test bus_test.c bus.c memory.c json.c ${CCAN_JSON})
target_compile_features(bus_test PRIVATE c_std_99)
target_link_libraries(bus_test PRIVATE cmocka-static)

add_executable(cpu_test cpu_test.c cpu.c vmarch.c bus.c memory.c ops.c ops.h json.c ${CCAN_JSON})
target_compile_features(cpu_test PRIVATE c_std_99)
target_link_libraries(cpu_test PRIVATE cmocka-static)

enable_testing()
add_test(NAME memory_test COMMAND memory_test)
add_test(NAME bus_test COMMAND bus_test)
add_test(NAME cpu_test COMMAND bus_test)
