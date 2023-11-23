cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(TEST_NAME "Test.BeThreading")
file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_be_executable(${TEST_NAME} "${SOURCES}")

target_link_libraries(${TEST_NAME} PUBLIC "BeThreading")

target_compile_definitions(${TEST_NAME} PRIVATE BE_TEST_THREADING)
target_compile_definitions(${TEST_NAME} PRIVATE BE_CURRENT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})