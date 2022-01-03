cmake_minimum_required(VERSION 3.20)
project(thm C)

include(cmake/FetchCMocka.cmake)

enable_testing()

add_subdirectory(vm)
add_subdirectory(tc)
