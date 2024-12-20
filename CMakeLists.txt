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

if (MSVC AND CMAKE_EXPORT_COMPILE_COMMANDS)
  # fake compile_commands.json generation from intermediate artifacts, since MSVC does not support it
  find_package(Python3 COMPONENTS Interpreter)
  add_custom_command(
    TARGET minekraf POST_BUILD
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/script/generate_compile_commands.py" "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" "${CMAKE_CURRENT_LIST_DIR}/compile_commands.json"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    VERBATIM)
endif()

install(
  TARGETS minekraf
  DESTINATION bin)
