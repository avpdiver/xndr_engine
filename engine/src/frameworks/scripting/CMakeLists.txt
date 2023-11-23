# BE SCRIPTING 

set(LIBRARY_NAME "BeScripting")
file(GLOB_RECURSE SOURCES "*.h" "*.cpp")
set(SOURCES "${ANGELSCRIPT_ADDONS};${SOURCES}")

add_be_static_lib(${LIBRARY_NAME} "${SOURCES}")

target_include_directories(${LIBRARY_NAME} PUBLIC "../.." ${ANGELSCRIPT_INCLUDE})
target_link_libraries(${LIBRARY_NAME} PUBLIC "BeBase" ${ANGELSCRIPT_LIB})

install(TARGETS ${LIBRARY_NAME} ARCHIVE DESTINATION "lib")