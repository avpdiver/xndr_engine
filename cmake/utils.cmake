function(add_be_static_lib TARGET_NAME SOURCES)
    add_library(${TARGET_NAME} STATIC ${SOURCES})
    
    # C++ standard
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD ${CXX_STANDARD_NUMBER} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
    target_compile_features(${TARGET_NAME} PRIVATE "cxx_std_${CXX_STANDARD_NUMBER}")

    # All configurations
    target_compile_definitions(${TARGET_NAME} PRIVATE ${PROJECT_DEFINES})

    # Release configuration
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>: ${PROJECT_DEFINES_RELEASE}>)
    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>: ${PROJECT_CXX_FLAGS_RELEASE}>)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE ${PROJECT_LINKER_FLAGS_RELEASE})

    # Debug configuration
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>: ${PROJECT_DEFINES_DEBUG}>)
    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>: ${PROJECT_CXX_FLAGS_DEBUG}>)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG ${PROJECT_LINKER_FLAGS_DEBUG})
endfunction()


function(add_be_executable TARGET_NAME SOURCES)

    message(STATUS "${TARGET_NAME} sources: ${SOURCES}")

    add_executable(${TARGET_NAME} ${SOURCES})

    # C++ standard
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD ${CXX_STANDARD_NUMBER} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
    target_compile_features(${TARGET_NAME} PRIVATE "cxx_std_${CXX_STANDARD_NUMBER}")

    # All configurations
    target_compile_definitions(${TARGET_NAME} PRIVATE ${PROJECT_DEFINES})

    # Release configuration
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>: ${PROJECT_DEFINES_RELEASE}>)
    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Release>: ${PROJECT_CXX_FLAGS_RELEASE}>)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE ${PROJECT_LINKER_FLAGS_RELEASE})

    # Debug configuration
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>: ${PROJECT_DEFINES_DEBUG}>)
    target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>: ${PROJECT_CXX_FLAGS_DEBUG}>)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG ${PROJECT_LINKER_FLAGS_DEBUG})
endfunction()