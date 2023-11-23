cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

include("cmake/externals.cmake")

add_subdirectory("src")

set(BE_ENABLE_ENGINE_TESTS TRUE)

if (BE_ENABLE_ENGINE_TESTS)
    add_subdirectory( "tests" )
endif()

add_subdirectory("tools")
