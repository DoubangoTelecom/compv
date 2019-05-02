cmake_minimum_required(VERSION 3.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

target_check_return_if_exist(compv_drawing)

project(libcompv_drawing VERSION 1.0.0 LANGUAGES CXX C)

include(ExternalProject)
add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../base/" compv_base)
add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../gl/" compv_gl)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../base/include
	${CMAKE_CURRENT_SOURCE_DIR}/../gl/include
	${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common/glm-0.9.8.2
)
if (WIN32)
	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common/sdl-2.0.5
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/include/common/glew-2.3.4
	)
endif()

if (WIN32)
	if ("${TARGET_ARCH}" STREQUAL "x64")
		set(LIB_WIN_DIR_SUFFIX win64)
	else ()
		set(LIB_WIN_DIR_SUFFIX win32)
	endif()
	link_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/libraries/windows/glew-2.3.4-${LIB_WIN_DIR_SUFFIX}
		${CMAKE_CURRENT_SOURCE_DIR}/../thirdparties/libraries/windows/sdl-2.0.5-${LIB_WIN_DIR_SUFFIX}-release
	)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_CXX_FLAGS} -DCOMPV_DRAWING_EXPORTS")
if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLEW_STATIC -DHAVE_GL_GLEW_H=1 -DHAVE_SDL_H=1")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_GL_DISABLE=1")
endif()

set(compv_drawing_SOURCES 
	###### root ######
	compv_drawing.cxx
	compv_drawing_canvas_skia.cxx
	compv_drawing_factory.cxx
	compv_drawing_image_libjpeg.cxx
	compv_drawing_window_egl.cxx
	compv_drawing_window_egl_android.cxx
	compv_drawing_window_glfw3.cxx
	compv_drawing_window_sdl.cxx
)

###### The library ######
add_library(compv_drawing ${LIB_BUILD_TYPE} ${compv_drawing_SOURCES})

###### 3rd parties libs ######
target_link_libraries(compv_drawing ${LIB_LINK_SCOPE} compv_base compv_gl)
add_dependencies(compv_drawing compv_base compv_gl)
if (WIN32)
  target_link_libraries(compv_drawing ${LIB_LINK_SCOPE} OpenGL32.lib Glu32.lib glew32s.lib SDL2.lib Winmm.lib imm32.lib version.lib)
endif()

if(NOT WIN32 AND ENABLE_SHARED)
  set_target_properties(compv_drawing PROPERTIES POSITION_INDEPENDENT_CODE 1)
endif()

###### Install Headers ######
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
)

###### Install Libs ######
install(TARGETS compv_drawing DESTINATION lib)