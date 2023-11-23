# BE RENDERER

set(LIBRARY_NAME "BeRenderer")
file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_be_static_lib(${LIBRARY_NAME} "${SOURCES}")

target_include_directories(${LIBRARY_NAME} PUBLIC "../.." ${KTX_INCLUDE} ${CGLTF_INCLUDE} ${MESHOPTIMIZER_INCLUDE})
target_link_libraries(${LIBRARY_NAME} PUBLIC "BeBase")
target_link_libraries(${LIBRARY_NAME} PUBLIC "BeThreading")
target_link_libraries(${LIBRARY_NAME} PUBLIC "BeRHI")
target_link_libraries(${LIBRARY_NAME} PUBLIC ${KTX_LIB})
target_link_libraries(${LIBRARY_NAME} PUBLIC ${MESHOPTIMIZER_LIB})

install(TARGETS ${LIBRARY_NAME} ARCHIVE DESTINATION "lib")