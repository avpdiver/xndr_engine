# BE HID  Human Input Device

set(LIBRARY_NAME "BeHID")
file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_be_static_lib(${LIBRARY_NAME} "${SOURCES}")

target_include_directories(${LIBRARY_NAME} PUBLIC "../..")
target_link_libraries(${LIBRARY_NAME} PUBLIC "BeBase")

install(TARGETS ${LIBRARY_NAME} ARCHIVE DESTINATION "lib")