cmake_minimum_required(VERSION 3.8.0)
project (VelEngine)


set (VelEngine_VERSION_MAJOR 0)
set (VelEngine_VERSION_MINOR 1)

option(USE_VELTEST "Adds executable test case" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

#Make function to set both include and source? then add only includes?
set(HEADERS
	${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/include/VelEng.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Renderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/VulkanTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/VulkanUtils.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Assets/Images.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Assets/GLTFObjectLoader.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Buffers/Buffers.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Buffers/GPUAllocator.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Frame/Frames.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Frame/Swapchain.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/DeferredPasses.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/Descriptors.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/GLTFMaterialPass.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/Pipeline.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/PipelineBuilder.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/ShadowPass.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/RenderPasses/SkyboxPass.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Scene/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Scene/Lighting.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Rendering/Scene/Renderable.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/UI/VelImgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/UI/GUIValue.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/Utils/DeletionQueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Utils/RenderThreadPool.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Utils/TSQueue.h
)

set(SOURCE
	${SOURCE}
	${CMAKE_CURRENT_SOURCE_DIR}/src/VelEng.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Renderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/VulkanUtils.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Assets/Images.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Assets/GLTFObjectLoader.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Buffers/GPUAllocator.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Frame/Frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Frame/Swapchain.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/DeferredPasses.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/Descriptors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/GLTFMaterialPass.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/Pipeline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/PipelineBuilder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/ShadowPass.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/RenderPasses/SkyboxPass.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Scene/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Scene/Lighting.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Scene/Renderable.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/UI/VelImgui.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/Utils/RenderThreadPool.cpp
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
