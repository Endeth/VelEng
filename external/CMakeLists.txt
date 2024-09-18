find_package(Vulkan REQUIRED)


add_subdirectory(fmt EXCLUDE_FROM_ALL)


add_library(vkbootstrap STATIC)
target_sources(vkbootstrap PRIVATE 
  vkbootstrap/VkBootstrap.h
  vkbootstrap/VkBootstrap.cpp
)

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)
set_property(TARGET vkbootstrap PROPERTY CXX_STANDARD 20)


add_library(sdl2 INTERFACE)
target_include_directories(sdl2 INTERFACE $ENV{VULKAN_SDK}/Include/SDL2 )
target_link_directories(sdl2 INTERFACE $ENV{VULKAN_SDK}/Lib )
target_link_libraries(sdl2 INTERFACE SDL2)


add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan sdl2)


add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)


add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)


add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb_image)


add_subdirectory(fastgltf)