cmake_minimum_required(VERSION 3.7.0)
project (VelEng)

set (VelEng_VERSION_MAJOR 0)
set (VelEng_VERSION_MINOR 1)

configure_file (
	"${PROJECT_SOURCE_DIR}/VelEngConfig.h.in"
	"${PROJECT_BINARY_DIR}/VelEngConfig.h"
)
 
#include_directories( ${VELUTI_INCLUDE_DIR} )
 
add_subdirectory (VelEng)
add_subdirectory (VelUti)
#include_directories("${PROJECT_BINARY_DIR}")
#nclude_directories("${PROJECT_BINARY_DIR}/Sqrt")

 
#add_executable(VelEng VelEng.cpp)
#target_link_libraries(VelEng ${EXTRA_LIBS})