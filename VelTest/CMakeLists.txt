
add_executable (VelTest ${CMAKE_CURRENT_SOURCE_DIR}/VelTest.cpp)
target_link_libraries(VelTest
	VelEng
	${CMAKE_CURRENT_SOURCE_DIR}/SOIL.lib
)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

