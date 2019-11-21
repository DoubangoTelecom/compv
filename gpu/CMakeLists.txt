cmake_minimum_required(VERSION 3.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

target_check_return_if_exist(compv_gpu)

project(libcompv_gpu VERSION 1.0.0 LANGUAGES CXX C)

include(ExternalProject)
add_contrib("${CMAKE_CURRENT_SOURCE_DIR}/../base/" compv_base)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../base/include
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_GPU_EXPORTS")

set(compv_gpu_SOURCES 
	###### base/math ######
	base/math/compv_gpu_math_convlt.cxx

	###### core/features/fast ######
	core/features/fast/compv_gpu_feature_fast_dete.cxx

	###### root ######
	compv_gpu.cxx
)

###### The library ######
add_library(compv_gpu ${LIB_BUILD_TYPE} ${compv_gpu_SOURCES})

###### 3rd parties libs ######
target_link_libraries(compv_gpu ${LIB_LINK_SCOPE} compv_base)
add_dependencies(compv_gpu compv_base)

if(NOT WIN32)
  set_target_properties(compv_gpu PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

###### Install Headers ######
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
)

###### Install Libs ######
install(TARGETS compv_gpu DESTINATION lib)