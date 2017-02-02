
add_executable (VelTest ${CMAKE_CURRENT_SOURCE_DIR}/VelTest.cpp)
target_link_libraries(VelTest
	VelEng
	${CMAKE_CURRENT_SOURCE_DIR}/SOIL.lib
	${CMAKE_CURRENT_SOURCE_DIR}/glfw3dll.lib
)
find_package(OpenGL REQUIRED)
target_link_libraries(VelTest ${OPENGL_LIBRARIES})
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(VelTest ${GLEW_LIBRARIES})

set(RESPATH ${CMAKE_CURRENT_SOURCE_DIR}/Resources)