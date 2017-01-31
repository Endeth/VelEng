cmake_minimum_required(VERSION 3.7.0)
project (VelEng)

set (VelEng_VERSION_MAJOR 0)
set (VelEng_VERSION_MINOR 1)


find_package(OpenGL REQUIRED)


configure_file (
	"${PROJECT_SOURCE_DIR}/VelEngConfig.h.in"
	"${PROJECT_BINARY_DIR}/VelEngConfig.h"
)
 
add_subdirectory(VelEng)
add_subdirectory(VelUti)
