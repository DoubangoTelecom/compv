cmake_minimum_required(VERSION 3.0)

project(libcompv_gl VERSION 1.0.0 LANGUAGES CXX C)

include(ExternalProject)
include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../base/" compv_base)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/contrib/include
	${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common/glm-0.9.8.2
)
if (WIN32)
	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common/glew-2.3.4
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common/freetype-2.8
	)
endif()

link_directories(
	${CMAKE_BINARY_DIR}/contrib/lib
)
if (WIN32)
	if ("${TARGET_ARCH}" STREQUAL "x64")
		set(LIB_WIN_DIR_SUFFIX win64)
	else ()
		set(LIB_WIN_DIR_SUFFIX win32)
	endif()
	link_directories(
		${LINK_DIRECTORIES}
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/libraries/windows/glew-2.3.4-${LIB_WIN_DIR_SUFFIX}
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/libraries/windows/freetype-2.8-${LIB_WIN_DIR_SUFFIX}
	)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_CXX_FLAGS} -DCOMPV_GL_EXPORTS")
if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_FREETYPE_FREETYPE_H=1 -DGLEW_STATIC -DHAVE_GL_GLEW_H=1")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_GL_DISABLE=1")
endif()

set(compv_gl_SOURCES ${compv_gl_SOURCES}
	###### drawing ######
	drawing/compv_gl_draw.cxx
	drawing/compv_gl_draw_lines.cxx
	drawing/compv_gl_draw_points.cxx
	drawing/compv_gl_draw_texts.cxx

	###### root ######
	compv_gl.cxx
	compv_gl_blitter.cxx
	compv_gl_canvas.cxx
	compv_gl_context.cxx
	compv_gl_fbo.cxx
	compv_gl_freetype.cxx
	compv_gl_info.cxx
	compv_gl_mvp.cxx
	compv_gl_program.cxx
	compv_gl_renderer.cxx
	compv_gl_surface.cxx
	compv_gl_surfacelayer_matching.cxx
	compv_gl_surfacelayer_multi.cxx
	compv_gl_surfacelayer_single.cxx
	compv_gl_utils.cxx
	compv_gl_vao.cxx
	compv_gl_window.cxx
)

###### The library ######
add_library(compv_gl ${LIB_BUILD_TYPE} ${compv_gl_SOURCES})

###### 3rd parties libs ######
target_link_libraries(compv_gl PRIVATE compv_base)
add_dependencies(compv_gl contrib_compv_base)
if (WIN32)
  target_link_libraries(compv_gl PRIVATE OpenGL32.lib Glu32.lib glew32s.lib freetype.lib)
endif()

if(NOT WIN32 AND ENABLE_SHARED)
  set_target_properties(compv_gl PROPERTIES POSITION_INDEPENDENT_CODE 1)
endif()

###### Install Headers ######
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
)

###### Install Libs ######
install(TARGETS compv_gl DESTINATION lib)