set(SOURCE ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/VelGL.cpp)
set(HEADERS ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/VelGL.h)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VBuffers)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VCameras)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VDrawable)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VGeo)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VLights)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VMaterials)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VOpenGL)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VRendering)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VShaders)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VTextures)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VWindow)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

add_library(VelEng ${SOURCE} ${HEADERS})

find_package(OpenGL REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VOpenGL/)
target_link_libraries(VelEng ${CMAKE_CURRENT_SOURCE_DIR}/VOpenGL/lib)

include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(VelEng ${OPENGL_LIBRARIES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VOpenGL/SOIL)
target_link_libraries(VelEng ${CMAKE_CURRENT_SOURCE_DIR}/VOpenGL/SOIL/)

source_group(VWindow FILES VWindow.cpp VWindowInfo.cpp)
