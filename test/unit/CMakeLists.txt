cmake_minimum_required(VERSION 3.10)

set (CMAKE_C_STANDARD 11)
project(constellation-tests C)
enable_testing()

add_compile_definitions(TESTING)

set(COMMON_SRC "../../src")
include_directories(${COMMON_SRC})

add_executable(selector_test selector_test.c ${COMMON_SRC}/shared.c ${COMMON_SRC}/selector.c )
target_link_libraries(selector_test PRIVATE cmocka)
add_test(NAME selector_test COMMAND selector_test)
