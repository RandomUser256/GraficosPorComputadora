cmake_minimum_required(VERSION 3.10.0)
project(GlassTracer VERSION 0.1.0 LANGUAGES C CXX)

# Add GLM
add_subdirectory(glm-master)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW or fetch it if not found
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    FetchContent_MakeAvailable(glfw)
endif()

# Find source files for ImGui
file(GLOB IMGUI_SOURCES
    "imgui/*.cpp"
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_opengl3.cpp"
)

# Add the executable
add_executable(GlassTracer 
    main.cpp
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(GlassTracer PRIVATE 
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
    ${CMAKE_SOURCE_DIR}/glm-master
    ${CMAKE_SOURCE_DIR}/stb-master
)

# Link libraries
target_link_libraries(GlassTracer PRIVATE 
    glm
    OpenGL::GL
    glfw
)

# Set C++ standard
set_target_properties(GlassTracer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
