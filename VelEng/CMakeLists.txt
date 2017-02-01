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
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VUti)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/VWindow)


add_library(VelEng ${SOURCE} ${HEADERS})
set(VelEngPath ${CMAKE_CURRENT_SOURCE_DIR})


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VOpenGL/)

include_directories(${OPENGL_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/VOpenGL/SOIL)


