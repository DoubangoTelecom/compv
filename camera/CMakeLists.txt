cmake_minimum_required(VERSION 3.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

target_check_return_if_exist(compv_camera)

project(libcompv_camera VERSION 1.0.0 LANGUAGES CXX C)

include(ExternalProject)
add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../base/" compv_base)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../base/include
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_CAMERA_EXPORTS")

set(compv_camera_SOURCES 
	###### android ######
	android/compv_camera_android.cxx

	###### root ######
	compv_camera.cxx
)

###### The library ######
add_library(compv_camera ${LIB_BUILD_TYPE} ${compv_camera_SOURCES})

###### 3rd parties libs ######
target_link_libraries(compv_camera ${LIB_LINK_SCOPE} compv_base)
add_dependencies(compv_camera compv_base)

if(NOT WIN32)
  set_target_properties(compv_camera PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

###### Install Headers ######
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
)

###### Install Libs ######
install(TARGETS compv_camera DESTINATION lib)