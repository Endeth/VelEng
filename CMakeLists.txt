cmake_minimum_required(VERSION 3.8.0)
project (VelEngine)


set (VelEngine_VERSION_MAJOR 0)
set (VelEngine_VERSION_MINOR 1)

option(USE_VELTEST "Adds executable test case" ON)

include_directories(${CMAKE_SOURCE_DIR}/VelEng/src)
include_directories(${CMAKE_SOURCE_DIR}/VelEng/external)

add_subdirectory(VelEng)

if(USE_VELTEST)
	add_subdirectory(VelTest)
endif()
