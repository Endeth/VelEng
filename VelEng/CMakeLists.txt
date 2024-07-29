set(HEADERS ${HEADERS} PARENT_SCOPE)
set(SOURCE ${SOURCE} PARENT_SCOPE)

add_subdirectory(src)
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
	"${PROJECT_SOURCE_DIR}/VelEng/VelEngConfig.h.in"
	"${PROJECT_BINARY_DIR}/VelEng/generated/config/VelEngConfig.h"
)
include_directories(${PROJECT_BINARY_DIR}/VelEng/generated/)
set(HEADERS ${HEADERS} ${PROJECT_BINARY_DIR}/VelEng/generated/config/VelEngConfig.h)

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
