cmake_minimum_required(VERSION 3.7.0)
project (VelEngine)

set (VelEngine_VERSION_MAJOR 0)
set (VelEngine_VERSION_MINOR 1)

option(USE_VELTEST "Adds executable test case" ON)




configure_file (
	"${PROJECT_SOURCE_DIR}/VelEngineConfig.h.in"
	"${PROJECT_BINARY_DIR}/VelEngineConfig.h"
)
 
add_subdirectory(VelEng)
add_subdirectory(VelUti)
if(USE_VELTEST)
	add_subdirectory(VelTest)
endif()
