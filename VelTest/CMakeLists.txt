
add_executable (VelTest ${CMAKE_CURRENT_SOURCE_DIR}/VelTest.cpp)
target_link_libraries(VelTest
	VelEng
	${LIBRARIES_DIR}/SOIL.lib
	${LIBRARIES_DIR}/glfw3dll.lib
	${LIBRARIES_DIR}/assimp.lib
	${LIBRARIES_DIR}/glew32.lib
	${OPENGL_LIBRARIES}
)

set(RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources)