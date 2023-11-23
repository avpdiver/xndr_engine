# BE RHI

set(LIBRARY_NAME "BeRHI")
file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_be_static_lib(${LIBRARY_NAME} "${SOURCES}")

target_include_directories(${LIBRARY_NAME} PUBLIC "../.."
                                                  ${Vulkan_INCLUDE_DIR}
                                                  ${Vulkan_INCLUDE_DIR}/vma)
target_link_libraries(${LIBRARY_NAME} PUBLIC "BeBase" 
                                             ${Vulkan_LIBRARY})

install(TARGETS ${LIBRARY_NAME} ARCHIVE DESTINATION "lib")