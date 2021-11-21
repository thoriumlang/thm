cmake_minimum_required(VERSION 3.20)
project(c_vm C)

set(CMAKE_C_STANDARD 11)
add_executable(c_vm
        main.c
        vmarch.h
        opts.c opts.h
        bus.c bus.h
        cpu.c cpu.h
        ops.c ops.h
        memory.c memory.h)

include(cmake/FetchCMocka.cmake)
add_executable(memory_test memory_test.c memory.c)
target_compile_features(memory_test PRIVATE c_std_99)
target_link_libraries(memory_test PRIVATE cmocka-static)

add_executable(bus_test bus_test.c bus.c memory.c)
target_compile_features(bus_test PRIVATE c_std_99)
target_link_libraries(bus_test PRIVATE cmocka-static)

add_executable(cpu_test cpu_test.c cpu.c bus.c memory.c ops.c ops.h)
target_compile_features(cpu_test PRIVATE c_std_99)
target_link_libraries(cpu_test PRIVATE cmocka-static)

enable_testing()
add_test(NAME memory_test COMMAND memory_test)
add_test(NAME bus_test COMMAND bus_test)
add_test(NAME cpu_test COMMAND bus_test)
