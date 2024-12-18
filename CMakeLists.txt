cmake_minimum_required(VERSION 3.13)

project(minekraf)

find_package(OpenGL REQUIRED)

add_subdirectory(lib)

add_executable(minekraf)
add_subdirectory(src)

target_link_libraries(minekraf
  PRIVATE
    OpenGL::GL
    imgui
    SDL3::Headers
    SDL3::SDL3-static)

install(
  TARGETS minekraf
  DESTINATION bin)
