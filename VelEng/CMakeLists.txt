set(SOURCE ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/VelGL.cpp)
set(HEADERS ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/VelGL.h)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/external)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vAssets)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vBuffers)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vCameras)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vDrawable)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vGeo)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vInput)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vLights)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vMaterials)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vOptions)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vRendering)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vShaders)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vTextures)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vUti)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vWindow)


add_library(VelEng ${SOURCE} ${HEADERS})
set(VelEngPath ${CMAKE_CURRENT_SOURCE_DIR})


