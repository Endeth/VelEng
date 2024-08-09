
set(SOURCE
	${SOURCE}
	${CMAKE_CURRENT_SOURCE_DIR}/VelEng.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/MeshLoader.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Renderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Deferred.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/VulkanUtils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Swapchain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Descriptors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Pipeline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/PipelineBuilder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/GPUAllocator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Renderable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Lighting.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Images.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/ui/VelImgui.cpp

	PARENT_SCOPE
)

set(HEADERS
	${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/VelEng.h

	${CMAKE_CURRENT_SOURCE_DIR}/rendering/MeshLoader.h

	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Renderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Deferred.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/VulkanTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/VulkanUtils.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Swapchain.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Descriptors.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Pipeline.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/PipelineBuilder.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/GPUAllocator.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Material.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Renderable.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Lighting.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/rendering/Images.h

	${CMAKE_CURRENT_SOURCE_DIR}/ui/VelImgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/ui/GUIValue.h

	${CMAKE_CURRENT_SOURCE_DIR}/utils/DeletionQueue.h

	PARENT_SCOPE
)