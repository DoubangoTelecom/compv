cmake_minimum_required(VERSION 3.0)

project(libcompv_camera VERSION 1.0.0 LANGUAGES CXX C)

include(ExternalProject)
include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../base/" compv_base)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../base/include
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_CXX_FLAGS} -DCOMPV_CAMERA_EXPORTS")

set(compv_camera_SOURCES 
	###### android ######
	android/compv_camera_android.cxx

	###### root ######
	compv_camera.cxx
)

###### The library ######
add_library(compv_camera ${LIB_BUILD_TYPE} ${compv_camera_SOURCES})

###### 3rd parties libs ######
target_link_libraries(compv_camera PRIVATE compv_base)
add_dependencies(compv_camera compv_base)

if(NOT WIN32 AND ENABLE_SHARED)
  set_target_properties(compv_camera PROPERTIES POSITION_INDEPENDENT_CODE 1)
endif()

###### Install Headers ######
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
)

###### Install Libs ######
install(TARGETS compv_camera DESTINATION lib)