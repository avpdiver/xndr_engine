# TRACY
include("cmake/tracy.cmake")

# ANGELSCRIPT
add_subdirectory("${MAIN_EXTERNALS_DIR}/angelscript/angelscript/projects/cmake" "angelscript")
target_compile_options(angelscript PRIVATE -Wno-deprecated)

set(ANGELSCRIPT_INCLUDE "${MAIN_EXTERNALS_DIR}/angelscript/angelscript/include"
                        "${MAIN_EXTERNALS_DIR}/angelscript/add_on")
set(ANGELSCRIPT_LIB angelscript)
file(GLOB_RECURSE ANGELSCRIPT_ADDONS "${MAIN_EXTERNALS_DIR}/angelscript/add_on/scriptstdstring/*.h"
                                     "${MAIN_EXTERNALS_DIR}/angelscript/add_on/scriptstdstring/*.cpp")

# VULKAN
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan include dir: ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan lib: ${Vulkan_LIBRARY}")

# GLFW
add_subdirectory("${MAIN_EXTERNALS_DIR}/glfw" "glfw")
set(GLFW_LIB glfw)

# CONCURRENT QUEUE
set(CONCURRENT_QUEUE_INCLUDE "${MAIN_EXTERNALS_DIR}/concurrentqueue")

# KTX
file(TO_CMAKE_PATH $ENV{KTX_SOFTWARE_PATH} KTX_SOFTWARE_PATH)
set(KTX_INCLUDE "${KTX_SOFTWARE_PATH}/include")
set(KTX_LIB "${KTX_SOFTWARE_PATH}/lib/ktx.lib")
message(STATUS "KTX lib: ${KTX_LIB}")
message(STATUS "KTX include: ${KTX_INCLUDE}")

# BASISU
include("cmake/basisu.cmake")

# CGLTF
set(CGLTF_INCLUDE "${MAIN_EXTERNALS_DIR}/cgltf")

# MESHOPTIMIZER
add_subdirectory("${MAIN_EXTERNALS_DIR}/meshoptimizer" "meshoptimizer")
set(MESHOPTIMIZER_INCLUDE "${MAIN_EXTERNALS_DIR}/meshoptimizer/src")
set(MESHOPTIMIZER_LIB meshoptimizer)