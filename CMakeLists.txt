cmake_minimum_required(VERSION 3.0)

project(compv VERSION 1.0.0 LANGUAGES CXX C)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/base)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/camera)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gpu)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gl)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/drawing)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)