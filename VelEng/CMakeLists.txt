set(SOURCE ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/VelGL.cpp)
set(HEADERS ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/VelGL.h)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/external)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vAssets)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vGeo)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vGraphics)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vInput)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vOptions)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vScripts)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vUti)

foreach(FILE ${SOURCE} ${HEADERS} )
	#convert source file to absolute
	get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
	# Get the directory of the absolute source file
	get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
	# Remove common directory prefix to make the group
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
	# Make sure we are using windows slashes
	string(REPLACE "/" "\\" GROUP "${GROUP}")
	# Group into "Source Files" and "Header Files"
		if ("${FILE}" MATCHES ".*\\.cpp")
		    set(GROUP "Source Files${GROUP}")
		elseif("${FILE}" MATCHES ".*\\.h")
		    set(GROUP "Header Files${GROUP}")
		endif()
	source_group("${GROUP}" FILES "${FILE}")
endforeach()

add_library(VelEng ${SOURCE} ${HEADERS})
set(VelEngPath ${CMAKE_CURRENT_SOURCE_DIR})

message( "Looking for Vulkan..." )
find_package(Vulkan)
if(VULKAN_FOUND)
	message( "Vulkan found" )
	target_link_libraries(VelEng Vulkan::Vulkan)
else(VULKAN_FOUND)
	message( "Vulkan not found" )
	target_link_libraries(VelEng ${VULKAN_LIB})
endif(VULKAN_FOUND)
target_compile_definitions(VelEng PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_compile_definitions(VelEng PRIVATE GLFW_EXPOSE_NATIVE_WIN32)

target_link_libraries(VelEng ${LIBRARIES_DIR}/x64/glfw3.lib )
target_link_libraries(VelEng ${LIBRARIES_DIR}/x64/liblua53.a )
