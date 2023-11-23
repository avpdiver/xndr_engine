cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(Bit VERSION 0.0.1 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(MAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "" FORCE)
set(MAIN_BINARY_DIR "${CMAKE_BINARY_DIR}/bin"     CACHE INTERNAL "" FORCE)
set(MAIN_EXTERNALS_DIR "${MAIN_SOURCE_DIR}/externals" CACHE INTERNAL "" FORCE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "installation directory")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${MAIN_BINARY_DIR}" CACHE INTERNAL "" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${MAIN_BINARY_DIR}" CACHE INTERNAL "" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MAIN_BINARY_DIR}" CACHE INTERNAL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "" FORCE)
set(CXX_STANDARD_NUMBER 23)

message(STATUS "MAIN_SOURCE_DIR: ${MAIN_SOURCE_DIR}")
message(STATUS "MAIN_BINARY_DIR: ${MAIN_BINARY_DIR}")

# find_program(CCACHE "ccache")
# if(CCACHE)
#     message(STATUS "CCACHE is found: ${CCACHE}")
#     set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
#     set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
# else(CCACHE)
#     message(STATUS "CCACHE is NOT found")
# endif(CCACHE)

include("cmake/compilers.cmake")
include("cmake/compiler_tests.cmake")
include("cmake/utils.cmake")

# engine dependencies
add_subdirectory("engine")


#----------------------------------------------------------
# BUILD ASSETS

add_custom_target(asset_shaders)

set(${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
file(GLOB_RECURSE SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp ${SHADER_DIR}/*.mesh ${SHADER_DIR}/*.task)

foreach(SHADER IN LISTS SHADERS)
    message(STATUS "Compile shander: ${SHADER}")
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(TARGET asset_shaders
                       COMMAND ${Vulkan_GLSLC_EXECUTABLE} -x glslang --target-env=vulkan1.3 --target-spv=spv1.6 -O ${SHADER} -o ${SHADER_DIR}/${FILENAME}.spv
                       DEPENDS ${SHADER}
                       COMMENT "Compiling ${FILENAME}"
                       VERBATIM)    
endForeach()

# add_dependencies(${PROJECT_NAME} asset_shaders)