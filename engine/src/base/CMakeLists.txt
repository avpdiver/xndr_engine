# BE BASE 

set(LIBRARY_NAME "BeBase")
file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_be_static_lib(${LIBRARY_NAME} "${SOURCES}")

target_include_directories(${LIBRARY_NAME} PUBLIC ".." 
                                                  ${TRACY_INCLUDE})
target_link_libraries(${LIBRARY_NAME} PUBLIC ${TRACY_LIB}
                                             ${GLFW_LIB})

install(TARGETS ${LIBRARY_NAME} ARCHIVE DESTINATION "lib")