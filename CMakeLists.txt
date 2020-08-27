cmake_minimum_required(VERSION 3.2.0)

project(opengl_sdk)

# use egl
add_executable(gpgpu ${CMAKE_CURRENT_SOURCE_DIR}/gpgpu.cpp)

target_link_libraries(gpgpu -lGLESv2 -lEGL -lGL)
