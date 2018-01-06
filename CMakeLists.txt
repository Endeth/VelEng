cmake_minimum_required(VERSION 3.7.0)
project (VelEngine)

set (VelEngine_VERSION_MAJOR 0)
set (VelEngine_VERSION_MINOR 1)

option(USE_VELTEST "Adds executable test case" ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(LIBRARIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

find_package(Vulkan)
IF (NOT Vulkan_FOUND)
	find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/lib)
	IF (Vulkan_LIBRARY)
		set(Vulkan_FOUND ON)
		MESSAGE("Using bundled Vulkan library version")
	ENDIF()
ENDIF()

configure_file (
	"${PROJECT_SOURCE_DIR}/VelEng/VelEngConfig.h.in"
	"${PROJECT_BINARY_DIR}/VelEng/Config/VelEngConfig.h"
)
 
include_directories(${CMAKE_SOURCE_DIR}/VelEng/)
include_directories(${PROJECT_BINARY_DIR}/VelEng/Config)

add_subdirectory(VelEng)
add_subdirectory(VelUti)
if(USE_VELTEST)
	add_subdirectory(VelTest)
	set(VELTEST_RESPATH ${VELTEST_RESPATH} "${CMAKE_SOURCE_DIR}/VelTest/Resources/")
endif()
