cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(init)

project(minekraf
  VERSION 0.0.1
  DESCRIPTION "Minecraft clone for university homework"
  HOMEPAGE_URL "https://github.com/hersi97/minekraf"
  LANGUAGES CXX)

find_package(Git)
if(Git_FOUND)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
    RESULT_VARIABLE STATUS
    OUTPUT_VARIABLE COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT STATUS EQUAL 0)
    message(WARN "Could not get git revision for project")
    set(COMMIT_ID "")
  else()
    string(PREPEND COMMIT_ID ".r") # .r12345678
  endif()
endif()

find_package(OpenGL REQUIRED)

add_subdirectory(lib)

add_executable(minekraf)
set_property(TARGET minekraf PROPERTY CXX_STANDARD 20)

target_link_libraries(minekraf
  PRIVATE
    OpenGL::GL
    imgui
    SDL3::Headers
    SDL3::SDL3-static
    spdlog::spdlog)

add_subdirectory(src)

if(MSVC AND CMAKE_EXPORT_COMPILE_COMMANDS)
  # fake compile_commands.json generation from intermediate artifacts,
  # since MSVC does not support it
  find_package(Python3 COMPONENTS Interpreter)
  add_custom_command(
    TARGET minekraf POST_BUILD
    COMMENT "Generating compile_commands.json"
    COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_LIST_DIR}/script/generate_compile_commands.py"
      "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND}
      -E create_symlink
      "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
      "${CMAKE_CURRENT_LIST_DIR}/compile_commands.json"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    VERBATIM)
endif()

# configure git hooks
function(install_git_hook hook)
  message(STATUS "  installing ${hook}")
  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/git/${hook}.in"
    "${CMAKE_SOURCE_DIR}/.git/hooks/${hook}")
endfunction()
if(NOT DEFINED DISABLE_GIT_HOOKS AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
  message(STATUS "Installing git hooks")
  install_git_hook("pre-commit")
  # additional "install_git_hook()"s go here
endif()

install(
  TARGETS minekraf
  DESTINATION bin)
