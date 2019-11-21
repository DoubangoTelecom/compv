cmake_minimum_required(VERSION 3.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

target_check_return_if_exist(compv_core)

project(libcompv_core VERSION 1.0.0 LANGUAGES CXX C ASM)

include(ExternalProject)
add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../base/" compv_base)
add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../gpu/" compv_gpu)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../base/include
	${CMAKE_CURRENT_SOURCE_DIR}/../gpu/include
)
if (WIN32)
	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common/ffmpeg-3.3.1
	)
endif()

if (WIN32)
	if ("${TARGET_ARCH}" STREQUAL "x64")
		set(LIB_WIN_DIR_SUFFIX win64)
	else ()
		set(LIB_WIN_DIR_SUFFIX win32)
	endif()
	link_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/libraries/windows/ffmpeg-3.3.1-lgpl-${LIB_WIN_DIR_SUFFIX}
	)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_CORE_EXPORTS")
if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_FFMPEG")
endif()

set(compv_core_SOURCES 
	###### calib ######
	calib/compv_core_calib_camera.cxx
	calib/compv_core_calib_homography.cxx
	calib/compv_core_calib_utils.cxx

	###### ccl ######
	ccl/compv_core_ccl_lmser.cxx
	ccl/compv_core_ccl_lmser_result.cxx
	ccl/compv_core_ccl_lsl.cxx
	ccl/compv_core_ccl_lsl_result.cxx

	###### features/edges ######
	features/edges/compv_core_feature_canny_dete.cxx
	features/edges/compv_core_feature_edge_dete.cxx

	###### features/hog ######
	features/hog/compv_core_feature_hog_std.cxx
	features/hog/compv_core_feature_hog_t.cxx

	###### features/fast ######
	features/fast/compv_core_feature_fast_dete.cxx

	###### features/hough ######
	features/hough/compv_core_feature_houghkht.cxx
	features/hough/compv_core_feature_houghsht.cxx

	###### features/orb ######
	features/orb/compv_core_feature_orb_desc.cxx
	features/orb/compv_core_feature_orb_dete.cxx

	###### matchers ######
	matchers/compv_core_core_matcher_flann.cxx
	matchers/compv_core_matcher_bruteforce.cxx

	###### video ######
	video/compv_core_video_reader_ffmpeg.cxx
	video/compv_core_video_writer_ffmpeg.cxx

	###### root ######
	compv_core.cxx
	compv_core_simd_globals.cxx
)

###### --- X86 / X64 --- ######
if("${TARGET_ARCH}" STREQUAL "x86" OR "${TARGET_ARCH}" STREQUAL "x64")
	set(INTRIN_FILES ${INTRIN_FILES}
		###### ccl/intrin/x86 ######
		ccl/intrin/x86/compv_core_ccl_lsl_intrin_avx2.cxx
		ccl/intrin/x86/compv_core_ccl_lsl_intrin_sse2.cxx
		ccl/intrin/x86/compv_core_ccl_lsl_intrin_ssse3.cxx

		###### features/edges/intrin/x86 ######
		features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_avx2.cxx
		features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_sse2.cxx
		features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_ssse3.cxx

		###### features/fast/intrin/x86 ######
		features/fast/intrin/x86/compv_core_feature_fast_dete_intrin_avx2.cxx
		features/fast/intrin/x86/compv_core_feature_fast_dete_intrin_sse2.cxx

		###### features/hog/intrin/x86 ######
		features/hog/intrin/x86/compv_core_feature_hog_common_norm_intrin_sse2.cxx
		features/hog/intrin/x86/compv_core_feature_hog_std_intrin_avx2.cxx
		features/hog/intrin/x86/compv_core_feature_hog_std_intrin_sse2.cxx

		###### features/hough/intrin/x86 ######
		features/hough/intrin/x86/compv_core_feature_houghkht_intrin_avx.cxx
		features/hough/intrin/x86/compv_core_feature_houghkht_intrin_avx2.cxx
		features/hough/intrin/x86/compv_core_feature_houghkht_intrin_sse2.cxx
		features/hough/intrin/x86/compv_core_feature_houghsht_intrin_avx2.cxx
		features/hough/intrin/x86/compv_core_feature_houghsht_intrin_sse2.cxx
		features/hough/intrin/x86/compv_core_feature_houghsht_intrin_sse41.cxx

		###### features/orb/intrin/x86 ######
		features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_avx2.cxx
		features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_fma3_avx2.cxx
		features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_sse2.cxx
		features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_sse41.cxx
	)

	###### ASM files ######
	set(ASM_FILES ${ASM_FILES}
		###### ccl/asm/x86 ######
		ccl/asm/x86/compv_core_ccl_lsl_x64.s
		ccl/asm/x86/compv_core_ccl_lsl_x64_avx.s
		ccl/asm/x86/compv_core_ccl_lsl_x64_sse.s

		###### features/edges/asm/x86 ######
		features/edges/asm/x86/compv_core_feature_canny_dete_x64_avx.s
		features/edges/asm/x86/compv_core_feature_canny_dete_x64_sse.s
		features/edges/asm/x86/compv_core_feature_canny_dete_x86_avx.s
		features/edges/asm/x86/compv_core_feature_canny_dete_x86_sse.s

		###### features/fast/asm/x86 ######
		features/fast/asm/x86/compv_core_feature_fast_dete_macros_x86_avx.s
		features/fast/asm/x86/compv_core_feature_fast_dete_macros_x86_sse.s
		features/fast/asm/x86/compv_core_feature_fast_dete_x64_avx.s
		features/fast/asm/x86/compv_core_feature_fast_dete_x64_sse.s
		features/fast/asm/x86/compv_core_feature_fast_dete_x86_avx.s
		features/fast/asm/x86/compv_core_feature_fast_dete_x86_sse.s

		###### features/hog/asm/x86 ######
		features/hog/asm/x86/compv_core_feature_hog_common_norm_x64_sse.s

		###### features/hough/asm/x86 ######
		features/hough/asm/x86/compv_core_feature_houghkht_x64_avx.s
		features/hough/asm/x86/compv_core_feature_houghkht_x64_sse.s
		features/hough/asm/x86/compv_core_feature_houghsht_x86_avx.s
		features/hough/asm/x86/compv_core_feature_houghsht_x86_sse.s

		###### features/orb/asm/x86 ######
		features/orb/asm/x86/compv_core_feature_orb_desc_x64_avx.s
		features/orb/asm/x86/compv_core_feature_orb_desc_x64_sse.s
		features/orb/asm/x86/compv_core_feature_orb_desc_x86_avx.s
		features/orb/asm/x86/compv_core_feature_orb_desc_x86_sse.s
	)

	###### Set options for ASM files ######
	set (YASM_ARGS ${YASM_ARGS} -i "${CMAKE_CURRENT_SOURCE_DIR}/../base/asm/x86")

###### --- ARM32 / ARM64 --- ######
elseif ("${TARGET_ARCH}" STREQUAL "arm32" OR "${TARGET_ARCH}" STREQUAL "arm64")
	set(INTRIN_FILES ${INTRIN_FILES}
		###### ccl/intrin/arm ######
		ccl/intrin/arm/compv_core_ccl_lsl_intrin_neon.cxx

		###### features/edges/intrin/arm ######
		features/edges/intrin/arm/compv_core_feature_canny_dete_intrin_neon.cxx

		###### features/fast/intrin/arm ######
		features/fast/intrin/arm/compv_core_feature_fast_dete_intrin_neon.cxx

		###### features/hog/intrin/arm ######
		features/hog/intrin/arm/compv_core_feature_hog_common_norm_intrin_neon.cxx
		features/hog/intrin/arm/compv_core_feature_hog_std_intrin_neon.cxx

		###### features/hough/intrin/arm ######
		features/hough/intrin/arm/compv_core_feature_houghkht_intrin_neon.cxx
		features/hough/intrin/arm/compv_core_feature_houghsht_intrin_neon.cxx

		###### features/orb/intrin/arm ######
		features/orb/intrin/arm/compv_core_feature_orb_desc_intrin_neon.cxx
	)

	###### ASM files ######
	set(ASM_FILES ${ASM_FILES}
		###### ccl/asm/arm ######
		ccl/asm/arm/compv_core_ccl_lsl_arm32.S
		ccl/asm/arm/compv_core_ccl_lsl_arm32_neon.S
		ccl/asm/arm/compv_core_ccl_lsl_arm64.S
		ccl/asm/arm/compv_core_ccl_lsl_arm64_neon.S

		###### features/edges/asm/arm ######
		features/edges/asm/arm/compv_core_feature_canny_dete_arm32_neon.S
		features/edges/asm/arm/compv_core_feature_canny_dete_arm64_neon.S

		###### features/fast/asm/arm ######
		features/fast/asm/arm/compv_core_feature_fast_dete_arm32_neon.S
		features/fast/asm/arm/compv_core_feature_fast_dete_arm64_neon.S
		features/fast/asm/arm/compv_core_feature_fast_dete_macros_neon.S

		###### features/hog/asm/arm ######
		features/hog/asm/arm/compv_core_feature_hog_common_norm_arm32_neon.S
		features/hog/asm/arm/compv_core_feature_hog_common_norm_arm64_neon.S

		###### features/hough/asm/arm ######
		features/hough/asm/arm/compv_core_feature_houghkht_arm32.S
		features/hough/asm/arm/compv_core_feature_houghkht_arm64.S

		###### features/orb/asm/arm ######
		features/orb/asm/arm/compv_core_feature_orb_desc_arm32.S
		features/orb/asm/arm/compv_core_feature_orb_desc_arm64.S
	)

	###### Set building rules for ASM files ######
	set_source_files_properties(${ASM_FILES} APPEND PROPERTIES LANGUAGE ASM COMPILE_FLAGS "${FLAGS_NEONFMA} -I${CMAKE_CURRENT_SOURCE_DIR}/../base/asm/arm -I${CMAKE_CURRENT_SOURCE_DIR}/features/fast/asm/arm")
endif()

###### Set FLAGS for intrin files ######
set_INTRIN_COMPILE_FLAGS (${INTRIN_FILES})

###### Declare ASM objects ######
set_ASM_OBJECTS(${ASM_FILES})

###### The library ######
add_library(compv_core ${LIB_BUILD_TYPE} ${compv_core_SOURCES} ${INTRIN_FILES} ${ASM_OBJECTS})

###### 3rd parties libs ######
target_link_libraries(compv_core ${LIB_LINK_SCOPE} compv_base compv_gpu)
add_dependencies(compv_core compv_base compv_gpu)
if (WIN32)
  target_link_libraries(compv_core ${LIB_LINK_SCOPE} ws2_32.lib Secur32.lib avformat.lib avcodec.lib avutil.lib swresample.lib swscale.lib)
endif()

if(NOT WIN32 AND ENABLE_SHARED)
  set_target_properties(compv_core PROPERTIES POSITION_INDEPENDENT_CODE 1)
endif()

###### Install Headers ######
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
)

###### Install Libs ######
install(TARGETS compv_core DESTINATION lib)