cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(TOOL_NAME "gltfconv")
file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_be_executable(${TOOL_NAME} "${SOURCES}")

target_include_directories(${TOOL_NAME} PRIVATE "../.." ${STB_INCLUDE} ${BASISU_INCLUDE} ${KTX_INCLUDE} ${CGLTF_INCLUDE} ${MESHOPTIMIZER_INCLUDE})

target_link_libraries(${TOOL_NAME} PRIVATE "BeRenderer")
target_link_libraries(${TOOL_NAME} PRIVATE ${BASISU_LIB})
target_link_libraries(${TOOL_NAME} PRIVATE ${KTX_LIB})
target_link_libraries(${TOOL_NAME} PRIVATE ${MESHOPTIMIZER_LIB})
target_link_libraries(${TOOL_NAME} PRIVATE ${Vulkan_LIBRARY})

target_compile_definitions(${TOOL_NAME} PRIVATE TOOL_NAME)
target_compile_definitions(${TOOL_NAME} PRIVATE BE_CURRENT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")