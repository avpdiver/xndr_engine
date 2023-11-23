option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory("${MAIN_EXTERNALS_DIR}/tracy" "tracy")

set(TRACY_INCLUDE "${MAIN_EXTERNALS_DIR}/tracy/public")
set(TRACY_LIB Tracy::TracyClient)
