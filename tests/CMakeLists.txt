cmake_minimum_required(VERSION 3.0)

enable_testing()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

find_library(CMOCKA_LIB NAMES cmocka PATHS /usr/local/lib)
set(STD_LIB m)

include_directories(../src)

add_executable(crc32 test_crc32.c)

add_test(NAME crc32 COMMAND crc32)

target_link_libraries(crc32 ${CMOCKA_LIB} ${STD_LIB})
