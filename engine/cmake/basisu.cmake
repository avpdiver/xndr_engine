set(BASISU_PROJECT "basisu")

set(BASISU_SOURCES  
    ${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_backend.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_basis_file.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_bc7enc.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_comp.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_enc.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_etc.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_frontend.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_gpu_texture.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_kernels_sse.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_opencl.cpp	
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_pvrtc1_4.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_resample_filters.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_resampler.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_ssim.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/basisu_uastc_enc.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/jpgd.cpp
	${MAIN_EXTERNALS_DIR}/basisu/encoder/pvpngreader.cpp
	${MAIN_EXTERNALS_DIR}/basisu/transcoder/basisu_transcoder.cpp
	${MAIN_EXTERNALS_DIR}/basisu/zstd/zstd.c
)

add_be_static_lib(${BASISU_PROJECT} "${BASISU_SOURCES}")

target_compile_options(${BASISU_PROJECT} PRIVATE -Wno-maybe-uninitialized
                                                 -Wno-stringop-overflow
                                                 -Wno-sign-compare
                                                 -Wno-shadow=compatible-local
                                                 -Wno-error=sign-compare
                                                 -Wno-error=shadow=compatible-local)

target_compile_definitions(${BASISU_PROJECT} PRIVATE BASISD_SUPPORT_KTX2_ZSTD=1
                                                     BASISU_SUPPORT_SSE=0)

if (NOT MSVC)
   target_link_libraries(${BASISU_PROJECT} m pthread)
endif()

set(BASISU_INCLUDE "${MAIN_EXTERNALS_DIR}/basisu")
set(BASISU_LIB basisu)