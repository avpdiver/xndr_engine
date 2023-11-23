
set(BE_COMPILER_DEFINITIONS "")

#------------------------------------------------------------------------------
message(STATUS "Performing Test STD_CACHELINESIZE_SUPPORTED")
set(STD_CACHELINESIZE_SUPPORTED_SRC 
	"#include <new>
	static constexpr size_t Align = std::hardware_destructive_interference_size;
	int main () {
		return 0;
	}"
)
try_compile(
	STD_CACHELINESIZE_SUPPORTED
	SOURCE_FROM_VAR          "main.cpp" STD_CACHELINESIZE_SUPPORTED_SRC
	CXX_STANDARD             ${CXX_STANDARD_NUMBER}
	CXX_STANDARD_REQUIRED    YES
)	
if (STD_CACHELINESIZE_SUPPORTED)
	message(STATUS "\t\t\t\tstd::hardware_destructive_interference_size is supported")
	set(BE_COMPILER_DEFINITIONS "${BE_COMPILER_DEFINITIONS}" "BE_CACHE_LINE=std::hardware_destructive_interference_size")
elseif ((${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") OR (${CMAKE_SYSTEM_NAME} STREQUAL "iOS"))
	message(STATUS "\t\t\t\tCPU cache line is 128")
	set(BE_COMPILER_DEFINITIONS "${BE_COMPILER_DEFINITIONS}" "BE_CACHE_LINE=128")
else ()
	message(STATUS "\t\t\t\tCPU cache line is 64")
	set(BE_COMPILER_DEFINITIONS "${BE_COMPILER_DEFINITIONS}" "BE_CACHE_LINE=64") # TODO
endif ()

#------------------------------------------------------------------------------
message(STATUS "Performing Test HAS_HASHFN_HashArrayRepresentation")
set(HAS_HASHFN_HashArrayRepresentation_SRC 
	"#include <functional>
	int main () {
		char buffer[128] = {};
		(void)(std::_Hash_array_representation(reinterpret_cast<const unsigned char*>(buffer), std::size(buffer)));
		return 0;
	}"
)
try_compile(
	HAS_HASHFN_HashArrayRepresentation
	SOURCE_FROM_VAR          "main.cpp" HAS_HASHFN_HashArrayRepresentation_SRC
	CXX_STANDARD             ${CXX_STANDARD_NUMBER}
	CXX_STANDARD_REQUIRED    YES
)
if (HAS_HASHFN_HashArrayRepresentation)
    message(STATUS "Success")
	set(BE_COMPILER_DEFINITIONS "${BE_COMPILER_DEFINITIONS}" "BE_HAS_HASHFN_HashArrayRepresentation")
endif ()

#------------------------------------------------------------------------------
message(STATUS "Performing Test HAS_HASHFN_Murmur2OrCityhash")
set(HAS_HASHFN_Murmur2OrCityhash_SRC 
	"#include <functional>
	int main () {
		char buffer[128] = {};
		(void)(std::__murmur2_or_cityhash<size_t>()(buffer, std::size(buffer)));
		return 0;
	}"
)
try_compile(
	HAS_HASHFN_Murmur2OrCityhash
	SOURCE_FROM_VAR          "main.cpp" HAS_HASHFN_Murmur2OrCityhash_SRC
	CXX_STANDARD             ${CXX_STANDARD_NUMBER}
	CXX_STANDARD_REQUIRED    YES
)
if (HAS_HASHFN_Murmur2OrCityhash)
	message(STATUS "Success")
	set(BE_COMPILER_DEFINITIONS "${BE_COMPILER_DEFINITIONS}" "BE_HAS_HASHFN_Murmur2OrCityhash")
endif ()

#------------------------------------------------------------------------------
message(STATUS "Performing Test HAS_HASHFN_HashBytes")
set(HAS_HASHFN_HashBytes_SRC 
	"#include <functional>
	int main () {
		char buffer[128] = {};
		(void)(std::_Hash_bytes(buffer, std::size(buffer), 0));
		return 0;
	}"
)
try_compile(
	HAS_HASHFN_HashBytes
	SOURCE_FROM_VAR          "main.cpp" HAS_HASHFN_HashBytes_SRC
	CXX_STANDARD             ${CXX_STANDARD_NUMBER}
	CXX_STANDARD_REQUIRED    YES
)
if (HAS_HASHFN_HashBytes)
	message(STATUS "\t\t\t\tSuccess")
	set(BE_COMPILER_DEFINITIONS "${BE_COMPILER_DEFINITIONS}" "BE_HAS_HASHFN_HashBytes")
endif ()

set(PROJECT_DEFINES ${PROJECT_DEFINES} ${BE_COMPILER_DEFINITIONS})