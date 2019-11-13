
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vCameras)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vDrawable)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vLights)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vMaterials)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vRendering)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vShaders)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vTextures)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vVulkan)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vWindow)

set(SOURCE
   ${SOURCE}
   PARENT_SCOPE
)
set(HEADERS
   ${HEADERS}
   PARENT_SCOPE
)