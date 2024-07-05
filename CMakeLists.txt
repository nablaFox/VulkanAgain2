cmake_minimum_required(VERSION 3.10)
project(VulkanAgain2)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths
set(SRC_PATH src)
set(BIN_PATH bin)
set(SHADERS_PATH shaders)
set(DEMO_PATH demos/general_demo)

# Include directories
include_directories(
    lib/glm
    lib/glfw/include
    lib/vkbootstrap
    lib/vma/include
    lib/stb_image
    lib/fastgltf
    lib/fastgltf/include
    lib/fmt/include
    lib/entt
)

# Source files
file(GLOB SRC_FILES
    "${SRC_PATH}/*.cpp"
    "${DEMO_PATH}/*.cpp"
    "lib/vkbootstrap/VkBootstrap.cpp"
)

# Shader files
file(GLOB SHADER_FILES
    "${SHADERS_PATH}/*.vert"
    "${SHADERS_PATH}/*.frag"
    "${SHADERS_PATH}/*.comp"
)

# Add executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/lib/glfw/src/libglfw3.a
    ${CMAKE_SOURCE_DIR}/lib/fmt/libfmt.a
    ${CMAKE_SOURCE_DIR}/lib/fastgltf/libfastgltf.a
    dl
    pthread
    X11
    Xxf86vm
    Xrandr
    Xi
    vulkan
)

# Custom command to compile shaders
foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT ${SHADERS_PATH}/${FILE_NAME}.spv
        COMMAND glslc ${SHADER} -o ${SHADERS_PATH}/${FILE_NAME}.spv
        DEPENDS ${SHADER}
    )
    list(APPEND SPV_FILES ${SHADERS_PATH}/${FILE_NAME}.spv)
endforeach()

# Add custom target for shaders
add_custom_target(shaders ALL DEPENDS ${SPV_FILES})

# Ensure shaders are built before the main executable
add_dependencies(${PROJECT_NAME} shaders)
