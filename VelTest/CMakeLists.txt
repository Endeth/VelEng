
add_executable (VelTest ${CMAKE_CURRENT_SOURCE_DIR}/VelTest.cpp)
target_link_libraries(VelTest
	VelEng
	${LIBRARIES_DIR}/SOIL.lib
	${LIBRARIES_DIR}/glfw3dll.lib
	${LIBRARIES_DIR}/assimp.lib
	${LIBRARIES_DIR}/glew32.lib
	${LIBRARIED_DIR}/vulkan-1.lib
	${OPENGL_LIBRARIES}
)

set(V_RESPATH ${V_RESPATH} "${CMAKE_CURRENT_SOURCE_DIR}/Resources")

configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/VelTestConfig.h.in"
	"${PROJECT_BINARY_DIR}/VelTest/Config/VelTestConfig.h"
)
configure_file (
	"${PROJECT_SOURCE_DIR}/dlls/assimp-vc140-mt.dll"
	"${PROJECT_BINARY_DIR}/VelTest/Debug/assimp-vc140-mt.dll"
	COPYONLY
)
configure_file (
	"${PROJECT_SOURCE_DIR}/dlls/glew32.dll"
	"${PROJECT_BINARY_DIR}/VelTest/Debug/glew32.dll"
	COPYONLY
)
configure_file (
	"${PROJECT_SOURCE_DIR}/dlls/glfw3.dll"
	"${PROJECT_BINARY_DIR}/VelTest/Debug/glfw3.dll"
	COPYONLY
)
include_directories(${PROJECT_BINARY_DIR}/VelTest/Config)