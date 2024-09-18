cmake_minimum_required(VERSION 3.8.0)
project (VelEngine)


set (VelEngine_VERSION_MAJOR 0)
set (VelEngine_VERSION_MINOR 1)

option(USE_VELTEST "Adds executable test case" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

set(HEADERS
	${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/include/VelEng.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/MeshLoader.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Renderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Frames.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/Pipeline.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/PipelineBuilder.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/DeferredPasses.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/ShadowPass.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/SkyboxPass.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/VulkanTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/VulkanUtils.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Swapchain.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Descriptors.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/GPUAllocator.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Material.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Renderable.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Lighting.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Images.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/UI/VelImgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/UI/GUIValue.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Utils/DeletionQueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Utils/RenderThreadPool.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Utils/TSQueue.h
)

set(SOURCE
	${SOURCE}
	${CMAKE_CURRENT_SOURCE_DIR}/src/VelEng.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/MeshLoader.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Renderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Frames.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/Pipeline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/PipelineBuilder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/DeferredPasses.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/ShadowPass.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/SkyboxPass.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/VulkanUtils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Swapchain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Descriptors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/GPUAllocator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Renderable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Lighting.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Images.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/ui/VelImgui.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RenderThreadPool.cpp
)

add_subdirectory(res)

foreach(FILE ${HEADERS} )
	get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
	get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/include" "" GROUP "${PARENT_DIR}")
	string(REPLACE "/" "\\" GROUP "${GROUP}")
	set(GROUP "VelEng${GROUP}")
	source_group("${GROUP}" FILES "${FILE}")
endforeach()

foreach(FILE ${SOURCE} )
	get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
	get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/src" "" GROUP "${PARENT_DIR}")
	string(REPLACE "/" "\\" GROUP "${GROUP}")
	set(GROUP "VelEng${GROUP}")
	source_group("${GROUP}" FILES "${FILE}")
endforeach()

find_package(Vulkan REQUIRED)

configure_file (
	"${PROJECT_SOURCE_DIR}/VelEngConfig.h.in"
	"${PROJECT_BINARY_DIR}/generated/config/VelEngConfig.h"
)
include_directories(${PROJECT_BINARY_DIR}/generated/)
set(HEADERS ${HEADERS} ${PROJECT_BINARY_DIR}/generated/config/VelEngConfig.h)
source_group("VelEng" FILES "${PROJECT_BINARY_DIR}/generated/config/VelEngConfig.h")

add_library(VelEng ${SOURCE} ${HEADERS})
set_property(TARGET VelEng PROPERTY CXX_STANDARD 20)

if(MSVC)
	target_compile_options(VelEng PRIVATE "/MP")
	#4244 assign type mismatch
	target_compile_options(VelEng PRIVATE "/wd 4244")
	#4267 argument type mismatch
	target_compile_options(VelEng PRIVATE "/wd 4267")
endif()

target_link_libraries(VelEng PUBLIC
	Vulkan::Vulkan
	vkbootstrap
	vma
	fmt::fmt
	glm
	sdl2
	imgui
	stb_image
	fastgltf::fastgltf
)

add_subdirectory(external)

set(VelEngPath ${CMAKE_CURRENT_SOURCE_DIR})

if(USE_VELTEST)
	add_subdirectory(tests)
endif()
