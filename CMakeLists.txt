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

	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/MeshLoader.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Renderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Frames.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Deferred.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/ShadowPass.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/SkyboxPass.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/VulkanTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/VulkanUtils.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Swapchain.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Descriptors.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Pipeline.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/PipelineBuilder.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/GPUAllocator.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Material.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Renderable.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Lighting.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/Images.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/ui/VelImgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/ui/GUIValue.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/utils/DeletionQueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/utils/RenderThreadPool.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/utils/TSQueue.h
)

set(SOURCE
	${SOURCE}
	${CMAKE_CURRENT_SOURCE_DIR}/src/VelEng.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/MeshLoader.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Renderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Frames.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Deferred.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/ShadowPass.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/SkyboxPass.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/VulkanUtils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Swapchain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Descriptors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Pipeline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/PipelineBuilder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/GPUAllocator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Renderable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Lighting.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/Images.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/ui/VelImgui.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RenderThreadPool.cpp
)

add_subdirectory(res)

foreach(FILE ${SOURCE} ${HEADERS} )
	get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
	get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
	string(REPLACE "/" "\\" GROUP "${GROUP}")
		if ("${FILE}" MATCHES ".*\\.cpp")
		    set(GROUP "Source Files${GROUP}")
		elseif("${FILE}" MATCHES ".*\\.h")
		    set(GROUP "Header Files${GROUP}")
		endif()
	source_group("${GROUP}" FILES "${FILE}")
endforeach()

find_package(Vulkan REQUIRED)

configure_file (
	"${PROJECT_SOURCE_DIR}/VelEngConfig.h.in"
	"${PROJECT_BINARY_DIR}/generated/config/VelEngConfig.h"
)
include_directories(${PROJECT_BINARY_DIR}/generated/)
set(HEADERS ${HEADERS} ${PROJECT_BINARY_DIR}/generated/config/VelEngConfig.h)

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
