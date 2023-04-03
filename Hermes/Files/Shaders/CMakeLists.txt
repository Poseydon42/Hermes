cmake_minimum_required(VERSION 3.24)

project(HermesShaders)

set(SHADERS
    forward_frag.glsl
    forward_vert.glsl
    fs_postprocessing_frag.glsl
    fs_ui_frag.glsl
    fs_ui_vert.glsl
    fs_vert.glsl
    irradiance_convolution.glsl
    light_culling.glsl
    load_equirectangular_frag.glsl
    precompute_brdf.glsl
    render_uniform_cube.glsl
    skybox_frag.glsl
    skybox_vert.glsl
    solid_color_frag.glsl
    specular_prefilter.glsl
)

set(BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Bin)
set(GLSLC_FLAGS -I ${CMAKE_CURRENT_SOURCE_DIR}/Common -I ${CMAKE_CURRENT_SOURCE_DIR}/../../Source/RenderingEngine -D_GLSL_ --target-env=vulkan1.2)

find_package(Vulkan REQUIRED)

if (${Vulkan_glslc_FOUND})
    foreach(shader_path ${SHADERS})
        cmake_path(GET shader_path FILENAME shader_name)
        set(shader_bin_path "${BINARY_DIR}/${shader_name}.spv")
        add_custom_command(
            OUTPUT ${shader_bin_path}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader_path} -o ${shader_bin_path} ${GLSLC_FLAGS}
            COMMENT "Compiling ${shader_path}"
            DEPENDS ${shader_path}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        list(APPEND shader_binaries_list ${shader_bin_path})
    endforeach()

    add_custom_target(
        HermesShaders ALL
        DEPENDS ${shader_binaries_list}
    )
else()
    message(WARNING "GLSL compiler executable cannot be located, shaders will not be compiled automatically")
endif()
