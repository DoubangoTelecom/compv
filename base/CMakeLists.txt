cmake_minimum_required(VERSION 3.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

target_check_return_if_exist(compv_base)

project(libcompv_base VERSION 1.0.0 LANGUAGES CXX C ASM)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common)
if (WIN32)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/windows)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_BASE_EXPORTS")

set(compv_base_SOURCES 
	###### android ######
	android/compv_android_cpu-features.cxx
	android/compv_android_dexclassloader.cxx
	android/compv_android_native_activity.cxx

	###### drawing ######
	drawing/compv_canvas.cxx
	drawing/compv_mvp.cxx
	drawing/compv_renderer.cxx
	drawing/compv_surface.cxx
	drawing/compv_surfacelayer.cxx
	drawing/compv_surfacelayer_matching.cxx
	drawing/compv_surfacelayer_multi.cxx
	drawing/compv_surfacelayer_single.cxx
	drawing/compv_viewport.cxx
	drawing/compv_window.cxx
	drawing/compv_window_registry.cxx

	###### image ######
	image/compv_image.cxx
	image/compv_image_conv_common.cxx
	image/compv_image_conv_hsv.cxx
	image/compv_image_conv_rgbfamily.cxx
	image/compv_image_conv_to_grayscale.cxx
	image/compv_image_conv_to_rgbx.cxx
	image/compv_image_conv_to_yuv444p.cxx
	image/compv_image_decoder.cxx
	image/compv_image_integral.cxx
	image/compv_image_remap.cxx
	image/compv_image_scale_bicubic.cxx
	image/compv_image_scale_bilinear.cxx
	image/compv_image_scale_pyramid.cxx
	image/compv_image_threshold.cxx
	image/compv_image_utils.cxx

	###### jsoncpp-1.8.4 ######
	jsoncpp-1.8.4/jsoncpp.cxx

	###### math ######
	math/compv_math.cxx
	math/compv_math_activation_functions.cxx
	math/compv_math_basic_ops.cxx
	math/compv_math_cast.cxx
	math/compv_math_clip.cxx
	math/compv_math_convlt.cxx
	math/compv_math_distance.cxx
	math/compv_math_dot.cxx
	math/compv_math_eigen.cxx
	math/compv_math_exp.cxx
	math/compv_math_gauss.cxx
	math/compv_math_histogram.cxx
	math/compv_math_lines.cxx
	math/compv_math_matrix.cxx
	math/compv_math_moments.cxx
	math/compv_math_morph.cxx
	math/compv_math_op_minmax.cxx
	math/compv_math_op_mul.cxx
	math/compv_math_op_sub.cxx
	math/compv_math_pca.cxx
	math/compv_math_scale.cxx
	math/compv_math_stats.cxx
	math/compv_math_stats_fit.cxx
	math/compv_math_stats_ransac.cxx
	math/compv_math_transform.cxx
	math/compv_math_trig.cxx
	math/compv_math_utils.cxx

	###### math/lmfit-6.1 ######
	math/lmfit-6.1/lmcurve.cxx
	math/lmfit-6.1/lmcurve_tyd.cxx
	math/lmfit-6.1/lmmin.cxx

	###### ml ######
	ml/compv_base_ml_knn.cxx
	ml/compv_base_ml_svm.cxx

	###### easyexif ######
	easyexif/easyexif.cxx

	###### ml/libsvm-322 ######
	ml/libsvm-322/libsvm.cxx

	###### ml/LightGBM-stable-f1f18dd ######
	# -- Not used --

	###### parallel ######
	parallel/compv_asynctask10.cxx
	parallel/compv_asynctask11.cxx
	parallel/compv_condvar.cxx
	parallel/compv_mutex.cxx
	parallel/compv_parallel.cxx
	
	parallel/compv_runnable.cxx
	parallel/compv_semaphore.cxx
	parallel/compv_thread.cxx
	parallel/compv_threaddisp.cxx
	parallel/compv_threaddisp10.cxx
	parallel/compv_threaddisp11.cxx
	parallel/compv_threaddisptbb.cxx

	###### pugixml-1.9 ######
	pugixml-1.9/pugixml.cxx

	###### tbbmalloc ###### 
	tbbmalloc/backend.cpp
	tbbmalloc/backref.cpp
	tbbmalloc/frontend.cpp
	tbbmalloc/large_objects.cpp
	tbbmalloc/tbbmalloc.cpp

	###### time ######
	time/compv_time.cxx
	time/compv_timer.cxx

	###### video ######
	video/compv_video_reader.cxx
	video/compv_video_writer.cxx

	###### root ######
	compv_base.cxx
	compv_base64.cxx
	compv_bind.cxx
	compv_bits.cxx
	compv_box.cxx
	compv_box_interestpoint.cxx
	compv_buffer.cxx
	compv_caps.cxx
	compv_ccl.cxx
	compv_common.cxx
	compv_cpu.cxx
	compv_debug.cxx
	compv_exif.cxx
	compv_features.cxx
	compv_fileutils.cxx
	compv_gradient_fast.cxx
	compv_jni.cxx
	compv_json.cxx
	compv_kernel.cxx
	compv_lock.cxx
	compv_mat.cxx
	compv_matchers.cxx
	compv_md5.cxx
	compv_mem.cxx
	compv_mem_pool_light.cxx
	compv_obj.cxx
	compv_patch.cxx
	compv_points_transform.cxx
	compv_sharedlib.cxx
	compv_simd_globals.cxx
	compv_uselocal.cxx
	compv_xml.cxx
	compv_yaml.cxx
)

###### --- iOS --- ######
if (IOS)
    set_source_files_properties(
        compv_cpu.cxx
        compv_fileutils.cxx
        PROPERTIES COMPILE_FLAGS "-x objective-c++"
    )
endif ()

###### --- X86 / X64 --- ######
if ("${TARGET_ARCH}" STREQUAL "x86" OR "${TARGET_ARCH}" STREQUAL "x64")
	set(INTRIN_FILES ${INTRIN_FILES}
		###### image/intrin/x86 ######
		image/intrin/x86/compv_image_conv_grayscale_intrin_ssse3.cxx
		image/intrin/x86/compv_image_conv_hsv_intrin_avx2.cxx
		image/intrin/x86/compv_image_conv_hsv_intrin_ssse3.cxx
		image/intrin/x86/compv_image_conv_rgbfamily_intrin_avx2.cxx
		image/intrin/x86/compv_image_conv_rgbfamily_intrin_sse2.cxx
		image/intrin/x86/compv_image_conv_rgbfamily_intrin_ssse3.cxx
		image/intrin/x86/compv_image_conv_to_rgbx_intrin_avx2.cxx
		image/intrin/x86/compv_image_conv_to_rgbx_intrin_sse2.cxx
		image/intrin/x86/compv_image_conv_to_rgbx_intrin_ssse3.cxx
		image/intrin/x86/compv_image_integral_intrin_ssse3.cxx
		image/intrin/x86/compv_image_remap_intrin_avx2.cxx
		image/intrin/x86/compv_image_remap_intrin_sse41.cxx
		image/intrin/x86/compv_image_scale_bicubic_intrin_avx2.cxx
		image/intrin/x86/compv_image_scale_bicubic_intrin_sse2.cxx
		image/intrin/x86/compv_image_scale_bicubic_intrin_sse41.cxx
		image/intrin/x86/compv_image_scale_bilinear_intrin_avx2.cxx
		image/intrin/x86/compv_image_scale_bilinear_intrin_sse41.cxx
		image/intrin/x86/compv_image_threshold_intrin_avx2.cxx
		image/intrin/x86/compv_image_threshold_intrin_sse2.cxx
		image/intrin/x86/compv_image_threshold_intrin_sse41.cxx

		###### intrin/x86 ######
		intrin/x86/compv_bits_intrin_sse2.cxx
		intrin/x86/compv_gradient_fast_intrin_sse2.cxx
		intrin/x86/compv_mem_intrin_avx.cxx
		intrin/x86/compv_mem_intrin_sse2.cxx
		intrin/x86/compv_mem_intrin_ssse3.cxx
		intrin/x86/compv_patch_intrin_avx2.cxx
		intrin/x86/compv_patch_intrin_sse2.cxx

		###### math/intrin/x86 ######
		math/intrin/x86/compv_math_activation_functions_intrin_avx2.cxx
		math/intrin/x86/compv_math_activation_functions_intrin_sse2.cxx
		math/intrin/x86/compv_math_activation_functions_intrin_sse41.cxx
		math/intrin/x86/compv_math_cast_intrin_sse2.cxx
		math/intrin/x86/compv_math_convlt_intrin_avx2.cxx
		math/intrin/x86/compv_math_convlt_intrin_sse2.cxx
		math/intrin/x86/compv_math_distance_intrin_avx.cxx
		math/intrin/x86/compv_math_distance_intrin_avx2.cxx
		math/intrin/x86/compv_math_distance_intrin_sse2.cxx
		math/intrin/x86/compv_math_distance_intrin_sse42.cxx
		math/intrin/x86/compv_math_dot_intrin_avx.cxx
		math/intrin/x86/compv_math_dot_intrin_sse2.cxx
		math/intrin/x86/compv_math_exp_intrin_avx2.cxx
		math/intrin/x86/compv_math_exp_intrin_sse2.cxx
		math/intrin/x86/compv_math_histogram_intrin_sse2.cxx
		math/intrin/x86/compv_math_matrix_intrin_avx.cxx
		math/intrin/x86/compv_math_matrix_intrin_sse2.cxx
		math/intrin/x86/compv_math_matrix_intrin_sse41.cxx
		math/intrin/x86/compv_math_morph_intrin_avx2.cxx
		math/intrin/x86/compv_math_morph_intrin_sse2.cxx
		math/intrin/x86/compv_math_op_minmax_intrin_sse2.cxx
		math/intrin/x86/compv_math_op_mul_intrin_sse2.cxx
		math/intrin/x86/compv_math_op_sub_intrin_avx.cxx
		math/intrin/x86/compv_math_op_sub_intrin_sse2.cxx
		math/intrin/x86/compv_math_scale_intrin_sse2.cxx
		math/intrin/x86/compv_math_stats_intrin_avx.cxx
		math/intrin/x86/compv_math_stats_intrin_sse2.cxx
		math/intrin/x86/compv_math_transform_intrin_avx.cxx
		math/intrin/x86/compv_math_transform_intrin_sse2.cxx
		math/intrin/x86/compv_math_trig_intrin_avx.cxx
		math/intrin/x86/compv_math_trig_intrin_sse2.cxx
		math/intrin/x86/compv_math_utils_intrin_avx2.cxx
		math/intrin/x86/compv_math_utils_intrin_sse2.cxx
		math/intrin/x86/compv_math_utils_intrin_sse41.cxx
		math/intrin/x86/compv_math_utils_intrin_ssse3.cxx

		###### ml/libsvm-322/intrin/x86 ######
		ml/libsvm-322/intrin/x86/compv_ml_libsvm-322_intrin_avx.cxx
		ml/libsvm-322/intrin/x86/compv_ml_libsvm-322_intrin_sse2.cxx
	)

	###### ASM files ######
	set(ASM_FILES ${ASM_FILES}
		###### asm/x86 ######
		asm/x86/compv_bits_x64_sse.s
		asm/x86/compv_bits_x86.s
		asm/x86/compv_common_x86.s
		asm/x86/compv_gradient_fast_x64_sse.s
		asm/x86/compv_mem_x64.s
		asm/x86/compv_mem_x64_avx.s
		asm/x86/compv_mem_x64_sse.s
		asm/x86/compv_mem_x86.s
		asm/x86/compv_mem_x86_avx.s
		asm/x86/compv_mem_x86_sse.s
		asm/x86/compv_patch_x64_avx.s
		asm/x86/compv_patch_x64_sse.s
		asm/x86/compv_patch_x86_avx.s
		asm/x86/compv_patch_x86_sse.s
		asm/x86/compv_utils_x86.s
		asm/x86/compv_vldx_vstx_macros_x86.s

		###### image/asm/x86 ######
		image/asm/x86/compv_image_conv_grayscale_x86_sse.s
		image/asm/x86/compv_image_conv_hsv_x64_avx.s
		image/asm/x86/compv_image_conv_hsv_x64_sse.s
		image/asm/x86/compv_image_conv_rgbfamily_x64_avx.s
		image/asm/x86/compv_image_conv_rgbfamily_x64_sse.s
		image/asm/x86/compv_image_conv_rgbfamily_x86_avx.s
		image/asm/x86/compv_image_conv_rgbfamily_x86_sse.s
		image/asm/x86/compv_image_conv_to_rgbx_x64_avx.s
		image/asm/x86/compv_image_conv_to_rgbx_x64_sse.s
		image/asm/x86/compv_image_scale_bilinear_x64_avx.s
		image/asm/x86/compv_image_scale_bilinear_x64_sse.s
		image/asm/x86/compv_image_scale_bilinear_x86_avx.s
		image/asm/x86/compv_image_scale_bilinear_x86_sse.s
		image/asm/x86/compv_image_threshold_x64_avx.s
		image/asm/x86/compv_image_threshold_x64_sse.s

		###### math/asm/x86 ######
		math/asm/x86/compv_math_cast_x64_sse.s
		math/asm/x86/compv_math_convlt_x64_avx.s
		math/asm/x86/compv_math_convlt_x64_sse.s
		math/asm/x86/compv_math_convlt_x86_avx.s
		math/asm/x86/compv_math_convlt_x86_sse.s
		math/asm/x86/compv_math_distance_x64.s
		math/asm/x86/compv_math_distance_x64_avx.s
		math/asm/x86/compv_math_distance_x64_sse.s
		math/asm/x86/compv_math_distance_x86_sse.s
		math/asm/x86/compv_math_dot_x64_avx.s
		math/asm/x86/compv_math_dot_x64_sse.s
		math/asm/x86/compv_math_exp_x64_avx.s
		math/asm/x86/compv_math_histogram_x64.s
		math/asm/x86/compv_math_histogram_x64_avx.s
		math/asm/x86/compv_math_histogram_x64_sse.s
		math/asm/x86/compv_math_matrix_x64_avx.s
		math/asm/x86/compv_math_matrix_x64_sse.s
		math/asm/x86/compv_math_matrix_x86_avx.s
		math/asm/x86/compv_math_matrix_x86_sse.s
		math/asm/x86/compv_math_morph_x64_avx.s
		math/asm/x86/compv_math_morph_x64_sse.s
		math/asm/x86/compv_math_op_mul_x64_avx.s
		math/asm/x86/compv_math_op_mul_x64_sse.s
		math/asm/x86/compv_math_op_sub_x64_avx.s
		math/asm/x86/compv_math_op_sub_x64_sse.s
		math/asm/x86/compv_math_scale_x64_avx.s
		math/asm/x86/compv_math_scale_x64_sse.s
		math/asm/x86/compv_math_stats_x64_sse.s
		math/asm/x86/compv_math_stats_x86_avx.s
		math/asm/x86/compv_math_stats_x86_sse.s
		math/asm/x86/compv_math_transform_x86_avx.s
		math/asm/x86/compv_math_transform_x86_sse.s
		math/asm/x86/compv_math_trig_x64_avx.s
		math/asm/x86/compv_math_trig_x64_sse.s
		math/asm/x86/compv_math_utils_x64_avx.s
		math/asm/x86/compv_math_utils_x64_sse.s
		math/asm/x86/compv_math_utils_x86.s
		math/asm/x86/compv_math_utils_x86_avx.s
		math/asm/x86/compv_math_utils_x86_sse.s
	)

	###### Set options for ASM files ######
	set (YASM_ARGS ${YASM_ARGS} -i "${CMAKE_CURRENT_SOURCE_DIR}/asm/x86")

###### --- ARM32 / ARM64 --- ######
elseif ("${TARGET_ARCH}" STREQUAL "arm32" OR "${TARGET_ARCH}" STREQUAL "arm64")
	set(INTRIN_FILES ${INTRIN_FILES}
		###### image/intrin/arm ######
		image/intrin/arm/compv_image_conv_grayscale_intrin_neon.cxx
		image/intrin/arm/compv_image_conv_hsv_intrin_neon.cxx
		image/intrin/arm/compv_image_conv_rgbfamily_intrin_neon.cxx
		image/intrin/arm/compv_image_conv_to_rgbx_intrin_neon.cxx
		image/intrin/arm/compv_image_remap_intrin_neon.cxx
		image/intrin/arm/compv_image_scale_bicubic_intrin_neon.cxx
		image/intrin/arm/compv_image_scale_bilinear_intrin_neon.cxx
		image/intrin/arm/compv_image_threshold_intrin_neon.cxx

		###### intrin/arm ######
		intrin/arm/compv_bits_intrin_neon.cxx
		intrin/arm/compv_gradient_fast_intrin_neon.cxx
		intrin/arm/compv_mem_intrin_neon.cxx
		intrin/arm/compv_patch_intrin_neon.cxx

		###### math/intrin/arm ######
		math/intrin/arm/compv_math_activation_functions_intrin_neon.cxx
		math/intrin/arm/compv_math_cast_intrin_neon.cxx
		math/intrin/arm/compv_math_convlt_intrin_neon.cxx
		math/intrin/arm/compv_math_distance_intrin_neon.cxx
		math/intrin/arm/compv_math_dot_intrin_neon.cxx
		math/intrin/arm/compv_math_exp_intrin_neon.cxx
		math/intrin/arm/compv_math_histogram_intrin_neon.cxx
		math/intrin/arm/compv_math_matrix_intrin_neon64.cxx
		math/intrin/arm/compv_math_morph_intrin_neon.cxx
		math/intrin/arm/compv_math_op_minmax_intrin_neon.cxx
		math/intrin/arm/compv_math_op_mul_intrin_neon.cxx
		math/intrin/arm/compv_math_op_sub_intrin_neon.cxx
		math/intrin/arm/compv_math_scale_intrin_neon.cxx
		math/intrin/arm/compv_math_stats_intrin_neon64.cxx
		math/intrin/arm/compv_math_transform_intrin_neon64.cxx
		math/intrin/arm/compv_math_trig_intrin_neon.cxx
		math/intrin/arm/compv_math_utils_intrin_neon.cxx
	)

	###### ASM files ######
	set(ASM_FILES ${ASM_FILES}
		###### asm/arm ######
		asm/arm/compv_bits_arm32_neon.S
		asm/arm/compv_bits_arm64_neon.S
		asm/arm/compv_common_arm.S
		asm/arm/compv_common_arm32.S
		asm/arm/compv_common_arm64.S
		asm/arm/compv_gradient_fast_arm32_neon.S
		asm/arm/compv_gradient_fast_arm64_neon.S
		asm/arm/compv_mem_arm32_neon.S
		asm/arm/compv_mem_arm64_neon.S
		asm/arm/compv_patch_arm32_neon.S
		asm/arm/compv_patch_arm64_neon.S
		asm/arm/compv_test_arm.S

		###### image/asm/arm ######
		image/asm/arm/compv_image_conv_grayscale_arm32_neon.S
		image/asm/arm/compv_image_conv_grayscale_arm64_neon.S
		image/asm/arm/compv_image_conv_hsv_arm32_neon.S
		image/asm/arm/compv_image_conv_hsv_arm64_neon.S
		image/asm/arm/compv_image_conv_rgbfamily_arm32_neon.S
		image/asm/arm/compv_image_conv_rgbfamily_arm64_neon.S
		image/asm/arm/compv_image_conv_to_rgbx_arm32_neon.S
		image/asm/arm/compv_image_conv_to_rgbx_arm64_neon.S
		image/asm/arm/compv_image_scale_bilinear_arm32_neon.S
		image/asm/arm/compv_image_scale_bilinear_arm64_neon.S
		image/asm/arm/compv_image_threshold_arm32_neon.S
		image/asm/arm/compv_image_threshold_arm64_neon.S

		###### math/asm/arm ######
		math/asm/arm/compv_math_cast_arm32_neon.S
		math/asm/arm/compv_math_cast_arm64_neon.S
		math/asm/arm/compv_math_convlt_arm32_neon.S
		math/asm/arm/compv_math_convlt_arm64_neon.S
		math/asm/arm/compv_math_distance_arm32_neon.S
		math/asm/arm/compv_math_distance_arm64_neon.S
		math/asm/arm/compv_math_dot_arm32_neon.S
		math/asm/arm/compv_math_dot_arm64_neon.S
		math/asm/arm/compv_math_exp_arm32_neon.S
		math/asm/arm/compv_math_exp_arm64_neon.S
		math/asm/arm/compv_math_histogram_arm32_neon.S
		math/asm/arm/compv_math_histogram_arm64_neon.S
		math/asm/arm/compv_math_matrix_arm32_neon.S
		math/asm/arm/compv_math_matrix_arm64_neon.S
		math/asm/arm/compv_math_morph_arm32_neon.S
		math/asm/arm/compv_math_morph_arm64_neon.S
		math/asm/arm/compv_math_op_mul_arm32_neon.S
		math/asm/arm/compv_math_op_mul_arm64_neon.S
		math/asm/arm/compv_math_op_sub_arm32_neon.S
		math/asm/arm/compv_math_op_sub_arm64_neon.S
		math/asm/arm/compv_math_scale_arm32_neon.S
		math/asm/arm/compv_math_scale_arm64_neon.S
		math/asm/arm/compv_math_stats_arm32_neon.S
		math/asm/arm/compv_math_stats_arm64_neon.S
		math/asm/arm/compv_math_transform_arm32_neon.S
		math/asm/arm/compv_math_transform_arm64_neon.S
		math/asm/arm/compv_math_trig_arm32_neon.S
		math/asm/arm/compv_math_trig_arm64_neon.S
		math/asm/arm/compv_math_utils_arm32_neon.S
		math/asm/arm/compv_math_utils_arm64_neon.S
	)

	###### Set building rules for ASM files ######
	set_source_files_properties(${ASM_FILES} APPEND PROPERTIES LANGUAGE ASM COMPILE_FLAGS "${FLAGS_NEONFMA} -I${CMAKE_CURRENT_SOURCE_DIR}/asm/arm")
endif()

###### Set FLAGS for intrin files ######
set_INTRIN_COMPILE_FLAGS (${INTRIN_FILES})

###### Declare ASM objects ######
set_ASM_OBJECTS(${ASM_FILES})

###### The library ######
add_library(compv_base ${LIB_BUILD_TYPE} ${compv_base_SOURCES} ${INTRIN_FILES} ${ASM_OBJECTS})

###### 3rd parties libs ######
if (WIN32)
	target_link_libraries(compv_base ${LIB_LINK_SCOPE} Shlwapi.lib Winmm.lib)
else ()
	target_link_libraries(compv_base ${LIB_LINK_SCOPE} dl pthread)
endif()

if(NOT WIN32)
  set_target_properties(compv_base PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

###### Install Headers ######
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
)

###### Install Libs ######
install(TARGETS compv_base DESTINATION lib)
