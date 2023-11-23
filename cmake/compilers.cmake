set(COMPILER_FLAGS "")

# detect target platform
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(TARGET_PLATFORM "LINUX" CACHE INTERNAL "" FORCE)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(TARGET_PLATFORM "WINDOWS" CACHE INTERNAL "" FORCE)	
else ()
	message(FATAL_ERROR "unsupported platform ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
endif ()

# detect target platform bits
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(TARGET_PLATFORM_BITS 64)
else ()
	message(FATAL_ERROR "unsupported platform bits!")
endif ()

# detect cpu architecture
if (MSVC)
    if (NOT DEFINED CMAKE_GENERATOR_PLATFORM)
        set(CMAKE_GENERATOR_PLATFORM "X64")
    endif()
    string(TOUPPER ${CMAKE_GENERATOR_PLATFORM} PLATFORM_NAME)
    if ((DEFINED PLATFORM_NAME) AND (NOT (PLATFORM_NAME STREQUAL "")))
        if (${PLATFORM_NAME} STREQUAL "ARM64")
	        set(TARGET_CPU_ARCH "ARM64")
        elseif (${PLATFORM_NAME} STREQUAL "X64")
	        set(TARGET_CPU_ARCH "X64")
        else()
	        message(FATAL_ERROR "unknown platform '${CMAKE_GENERATOR_PLATFORM}'")
        endif()
    else()
        set(TARGET_CPU_ARCH "X64" CACHE INTERNAL "" FORCE)
    endif()
else()
	string(TOUPPER ${CMAKE_SYSTEM_PROCESSOR} SYSTEM_PROCESSOR)
	if (${SYSTEM_PROCESSOR} STREQUAL "AMD64")
		set(TARGET_CPU_ARCH "X64")
	elseif (${SYSTEM_PROCESSOR} STREQUAL "X86_64")
		set(TARGET_CPU_ARCH "X64")
	elseif (${PLATFORM_NAME} STREQUAL "AARCH64")
        set(TARGET_CPU_ARCH "ARM64")
    elseif (${PLATFORM_NAME} STREQUAL "ARM64")
        set(TARGET_CPU_ARCH "ARM64")		
	else()
		message(FATAL_ERROR "unknown processor '${CMAKE_SYSTEM_PROCESSOR}'")
	endif()
endif()

# Build config:
#	- Release  -- enable all optimizations
#	- Debug    -- enable all checks

set(CMAKE_CONFIGURATION_TYPES Release Debug)
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Configurations" FORCE)

set(PROJECT_DEFINES ${BE_COMPILER_DEFINITIONS}
                            "BE_PLATFORM_${TARGET_PLATFORM}" 
	                        "BE_PLATFORM_NAME=\"${CMAKE_SYSTEM_NAME}\""
	                        "BE_PLATFORM_BITS=${TARGET_PLATFORM_BITS}"
                            "BE_CPU_ARCH_${TARGET_CPU_ARCH}"
	                        "BE_CPU_ARCH_NAME=\"${TARGET_CPU_ARCH}\""
                            "_POSIX")
set(PROJECT_DEFINES_RELEASE "BE_RELEASE")
set(PROJECT_DEFINES_DEBUG   "BE_DEBUG")
	
#==================================================================================================
# MSVC Compilation settings
#==================================================================================================
string(FIND "${CMAKE_CXX_COMPILER_ID}" "MSVC" outPos)
if ((outPos GREATER -1))
    if (DEFINED DETECTED_COMPILER)
        message(FATAL_ERROR "multiple compiler types detected, previous: '${DETECTED_COMPILER}'")
    endif()
	
    set(DETECTED_COMPILER "COMPILER_MSVC")

    set(CURRENT_C_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "" FORCE)
    set(CURRENT_CXX_FLAGS ${CMAKE_CXX_FLAGS} CACHE STRING "" FORCE)
    set(CURRENT_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "" FORCE)
    set(CURRENT_STATIC_LINKER_FLAGS ${CMAKE_STATIC_LINKER_FLAGS} CACHE STRING "" FORCE)
    set(CURRENT_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} CACHE STRING "" FORCE)

    set(CURRENT_C_FLAGS "${CURRENT_C_FLAGS}")
    set(CURRENT_CXX_FLAGS "${CURRENT_CXX_FLAGS} /EHsc")

    #--------------------------------------------
    set(CONFIGURATION_DEPENDENT_PATH ON CACHE INTERNAL "" FORCE)

    #--------------------------------------------
    if (NOT CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        message(FATAL_ERROR "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION is not defined, Windows SDK is required")
    endif()

    set(MSVC_WARNING_LIST
        # errors
        /we4002 /we4099 /we4129 /we4130 /we4172 /we4238 /we4239 /we4240 /we4251 /we4263 /we4264 /we4266 /we4273 /we4293
        /we4305 /we4390 /we4455 /we4456 /we4457 /we4458 /we4459 /we4473 /we4474 /we4522 /we4552 /we4553 /we4554 /we4700 /we4706 /we4715 /we4716 /we4717
        /we4927 /w14834 /we5062 /we5054 /we4565 /we5054 /we4291 /we4297 /we4584
        # disable warnings
        /wd4061 /wd4062 /wd4063 /wd4310 /wd4324 /wd4365 /wd4503 /wd4514 /wd4530 /wd4623 /wd4625 /wd4626 /wd4710 /wd4714 /wd5026 /wd5027 /wd4201
   )
    if (${BE_ENABLE_COMPILER_WARNINGS})
        set(MSVC_WARNING_LIST ${MSVC_WARNING_LIST}
            # warnings
            /w14018 /w14127 /w14189 /w14244 /w14245 /w14287 /w14389 /w14505 /w14668 /w14701 /w14702 /w14703 /w14838 /w14946 /w14996 /w15038
       )
    else()
        set(MSVC_WARNING_LIST ${MSVC_WARNING_LIST}
            # disable warnings
            /wd4267 /wd4100 /wd4127 /wd4996
       )
    endif()

    set(MSVC_OPTS /std:c++latest /MP /Gm- /Zc:inline /Gy- /fp:strict /fp:except- /fsanitize=fuzzer /fsanitize=address
        ${COMPILER_FLAGS} ${MSVC_WARNING_LIST})

	set(PROJECT_DEFINES ${PROJECT_DEFINES}
        "BE_COMPILER_MSVC"
        "UNICODE=1")

    set(MSVC_OPTS_DBG ${MSVC_OPTS})
    if (${BE_ENABLE_COMPILER_WARNINGS})
        list(APPEND MSVC_OPTS_DBG /w14100)
    endif()

    set(CMAKE_CXX_FLAGS "${CURRENT_CXX_FLAGS}" CACHE STRING "" FORCE)

    # Release
    set(CMAKE_C_FLAGS_RELEASE "${CURRENT_C_FLAGS} /D_NDEBUG /DNDEBUG /MT /Ox /MP " CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CURRENT_CXX_FLAGS} /D_NDEBUG /DNDEBUG /MT /Ox /MP " CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CURRENT_EXE_LINKER_FLAGS} /LTCG /RELEASE " CACHE STRING "" FORCE)
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CURRENT_STATIC_LINKER_FLAGS} /LTCG /RELEASE " CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CURRENT_SHARED_LINKER_FLAGS} /LTCG /RELEASE " CACHE STRING "" FORCE)
    set(PROJECT_CXX_FLAGS_RELEASE ${MSVC_OPTS} /Ob2 /Oi /Ot /Oy /GT /GL /GF /GS- /W3 /Ox /analyze- CACHE INTERNAL "" FORCE)
    set(PROJECT_LINKER_FLAGS_RELEASE " /OPT:REF /OPT:ICF /INCREMENTAL:NO /LTCG /RELEASE /DYNAMICBASE" CACHE INTERNAL "" FORCE)

    # Debug
    set(CMAKE_C_FLAGS_DEBUG "${CURRENT_C_FLAGS} /D_DEBUG /MTd /Od /MP " CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CURRENT_CXX_FLAGS} /D_DEBUG /MTd /Od /Zi /MP " CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CURRENT_EXE_LINKER_FLAGS} /DEBUG:FULL " CACHE STRING "" FORCE)
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CURRENT_STATIC_LINKER_FLAGS} /DEBUG:FULL " CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CURRENT_SHARED_LINKER_FLAGS} /DEBUG:FULL " CACHE STRING "" FORCE)
    set(PROJECT_CXX_FLAGS_DEBUG ${MSVC_OPTS_DBG}  /W4 /WX- /sdl /Od /Ob0 /Oy- /GF- /GS /GR /analyze- /Zi /RTC1 CACHE INTERNAL "" FORCE)
    set(PROJECT_LINKER_FLAGS_DEBUG " /OPT:REF /OPT:ICF /INCREMENTAL:NO /DEBUG:FULL" CACHE INTERNAL "" FORCE)
endif()

#==================================================================================================
# GCC Compilation settings
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
#==================================================================================================
string(FIND "${CMAKE_CXX_COMPILER_ID}" "GNU" outPos)
if ((outPos GREATER -1))
    if (DEFINED DETECTED_COMPILER)
        message(FATAL_ERROR "multiple compiler types detected, previous: '${DETECTED_COMPILER}'")
    endif()

	set(DETECTED_COMPILER "COMPILER_GCC")

    #--------------------------------------------
	set(CONFIGURATION_DEPENDENT_PATH OFF CACHE INTERNAL "" FORCE)

	# -Wno-shadow -Wno-enum-compare -Wno-narrowing -Wno-attributes 
	set(GCC_OPTS ${COMPILER_FLAGS} -Wmaybe-uninitialized -Wfree-nonheap-object -Wcast-align -Wlogical-op -Waddress -Wno-non-template-friend -Werror=return-local-addr -Werror=placement-new -Werror=sign-compare -Werror=literal-suffix -Werror=shadow=local -Werror=delete-incomplete -Werror=odr -Werror=subobject-linkage -Werror=multichar -Winvalid-offsetof ${GCC_CLANG_SHARED_LOCAL_WARNING_LIST})
    
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(GCC_OPTS ${GCC_OPTS} -fsanitize=address)
    endif()

	set(PROJECT_DEFINES ${PROJECT_DEFINES} "BE_COMPILER_GCC")
 
	 # Release  TODO: -Ofast ?
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Release>: >)
	set(CMAKE_C_FLAGS_RELEASE "-O3 -finline-functions ${GCC_CLANG_SHARED_GLOBAL_WARNING_LIST}" CACHE STRING "" FORCE)
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -finline-functions ${GCC_CLANG_SHARED_GLOBAL_WARNING_LIST}" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CURRENT_EXE_LINKER_FLAGS} " CACHE STRING "" FORCE)
	set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CURRENT_STATIC_LINKER_FLAGS} " CACHE STRING "" FORCE)
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CURRENT_SHARED_LINKER_FLAGS} " CACHE STRING "" FORCE)
	set(PROJECT_CXX_FLAGS_RELEASE  ${GCC_OPTS} -O3 -Ofast -fomit-frame-pointer -finline-functions CACHE INTERNAL "" FORCE)
	set(PROJECT_LINKER_FLAGS_RELEASE " -static-libgcc -static-libstdc++ -static-libasan" CACHE INTERNAL "" FORCE)
	
	# Debug
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>: >)
	set(CMAKE_C_FLAGS_DEBUG "-O0 ${GCC_CLANG_SHARED_GLOBAL_WARNING_LIST}" CACHE STRING "" FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 ${GCC_CLANG_SHARED_GLOBAL_WARNING_LIST}" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CURRENT_EXE_LINKER_FLAGS} " CACHE STRING "" FORCE)
	set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CURRENT_STATIC_LINKER_FLAGS} " CACHE STRING "" FORCE)
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CURRENT_SHARED_LINKER_FLAGS} " CACHE STRING "" FORCE)
	set(PROJECT_CXX_FLAGS_DEBUG  ${GCC_OPTS} -g -ggdb -O0 -Wno-terminate CACHE INTERNAL "" FORCE)
	set(PROJECT_LINKER_FLAGS_DEBUG " -static-libgcc -static-libstdc++ -static-libasan" CACHE INTERNAL "" FORCE)
endif()

if (DEFINED DETECTED_COMPILER)
    message(STATUS "DETECTED_COMPILER: ${DETECTED_COMPILER}")
else()
    message(FATAL_ERROR "DETECTED_COMPILER: '${CMAKE_CXX_COMPILER_ID}' is not configured for this project!")
endif()

#----------------------------------------------------------------------------------------

message(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_STATIC_LINKER_FLAGS: ${CMAKE_STATIC_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

set(PROJECT_DEFINES         ${PROJECT_DEFINES}         CACHE INTERNAL "" FORCE)
set(PROJECT_DEFINES_RELEASE ${PROJECT_DEFINES_RELEASE} CACHE INTERNAL "" FORCE)
set(PROJECT_DEFINES_DEBUG   ${PROJECT_DEFINES_DEBUG}   CACHE INTERNAL "" FORCE)

message(STATUS "TARGET_PLATFORM: ${TARGET_PLATFORM}")
message(STATUS "TARGET_PLATFORM_BITS: ${TARGET_PLATFORM_BITS}")
message(STATUS "TARGET_CPU_ARCH: ${TARGET_CPU_ARCH}")
message(STATUS "PROJECT_DEFINES: ${PROJECT_DEFINES}")
message(STATUS "PROJECT_CXX_FLAGS_RELEASE: ${PROJECT_CXX_FLAGS_RELEASE}")
message(STATUS "PROJECT_CXX_FLAGS_DEBUG: ${PROJECT_CXX_FLAGS_DEBUG}")