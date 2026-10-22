cmake_minimum_required(VERSION 4.3)
project(rimg LANGUAGES C CXX)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(rimg)
target_sources(rimg
    PRIVATE
    src/main.cpp
    external/glad/src/glad.c
    src/stb_image_impl.cpp

    PUBLIC
    external/stb_image.h
    external/glad/include/glad/glad.h
)

target_link_libraries(rimg
    glfw
    OpenGL::GL
)

set_target_properties(rimg
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
