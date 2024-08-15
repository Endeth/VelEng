find_program(GLSL_VALIDATOR glslangValidator HINTS $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp"
    )

foreach(GLSL_SRC ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL_SRC} NAME)
  set(SPIRV_OUT "${CMAKE_CURRENT_SOURCE_DIR}/shaders/spv/${FILE_NAME}.spv")
  message(STATUS ${GLSL_SRC})
  message(STATUS COMMAND ${GLSL_VALIDATOR} " " -V " " ${GLSL_SRC} " " -o " " ${SPIRV_OUT})
  add_custom_command(
    OUTPUT ${SPIRV_OUT}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL_SRC} -o ${SPIRV_OUT}
    DEPENDS ${GLSL_SRC})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})
endforeach(GLSL_SRC)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

set(SHADERS_PATH_SPV ${CMAKE_CURRENT_SOURCE_DIR}/shaders/spv/ PARENT_SCOPE)
set(MESH_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mesh/ PARENT_SCOPE)
set(TEXTURE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/textures/ PARENT_SCOPE)