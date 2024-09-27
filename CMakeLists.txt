cmake_minimum_required(VERSION 3.8.0)
project (VelEngine)


set (VelEngine_VERSION_MAJOR 0)
set (VelEngine_VERSION_MINOR 1)

option(USE_VELTEST "Adds executable test case" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

set (FILE_TYPE_FOLDER "/include")
set(HEADERS
	${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/VelEng.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Actors/Actor.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Actors/Component.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Actors/RenderComponent.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Renderer.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/VulkanTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/VulkanUtils.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Assets/Images.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Assets/GLTFObjectLoader.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Buffers/Buffers.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Buffers/GPUAllocator.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Frame/Frames.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Frame/Swapchain.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/DeferredPasses.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/Descriptors.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/GLTFMaterialPass.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/Pipeline.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/PipelineBuilder.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/ShadowPass.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/SkyboxPass.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/Lighting.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/Renderable.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/RenderTarget.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/UI/VelImgui.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/UI/GUIValue.h

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Utils/DeletionQueue.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Utils/RenderThreadPool.h
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Utils/TSQueue.h
)

set (FILE_TYPE_FOLDER "/src")
set(SOURCE
	${SOURCE}
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/VelEng.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Actors/Actor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Actors/Component.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Actors/RenderComponent.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Renderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/VulkanUtils.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Assets/Images.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Assets/GLTFObjectLoader.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Buffers/GPUAllocator.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Frame/Frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Frame/Swapchain.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/DeferredPasses.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/Descriptors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/GLTFMaterialPass.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/Pipeline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/PipelineBuilder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/ShadowPass.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/RenderPasses/SkyboxPass.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/Lighting.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/Renderable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Rendering/Scene/RenderTarget.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/UI/VelImgui.cpp

	${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}/Utils/RenderThreadPool.cpp
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
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}${FILE_TYPE_FOLDER}" "" GROUP "${PARENT_DIR}")
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
