# Download and unpack sdl3 at configure time
set(SDL3_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/sdl3-prefix")
set(SDL3_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/sdl3")
configure_file("cmake/sdl3-dl/CMakeLists.txt.in" "sdl3-dl/CMakeLists.txt")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sdl3-dl")
execute_process(
  COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sdl3-dl")

set(SDL_STATIC ON CACHE BOOL "Build a static version of the library" FORCE)
add_subdirectory(sdl3)

# Download and unpack imgui at configure time
set(IMGUI_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/imgui-prefix")
set(IMGUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/imgui")
configure_file("cmake/imgui-dl/CMakeLists.txt.in" "imgui-dl/CMakeLists.txt")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imgui-dl")
execute_process(
  COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imgui-dl")

# Generate CMakeLists file for imgui
file(READ "${IMGUI_SOURCE_DIR}/imgui.h" imgui_ver)
string(REGEX MATCH "IMGUI_VERSION *\"([0-9](\.[0-9]+)*) *WIP\"" _ ${imgui_ver})
if(CMAKE_MATCH_1)
  set(IMGUI_VERSION ${CMAKE_MATCH_1})
else()
  set(IMGUI_VERSION "")
endif()
get_target_property(sdl_includes SDL3::Headers INTERFACE_INCLUDE_DIRECTORIES)
set(IMGUI_TEST_FLAGS_INCLUDE_DIRS "${sdl_includes}" CACHE INTERNAL "" FORCE)
configure_file(
  "cmake/imgui/CMakeLists.txt.in"
  "${IMGUI_SOURCE_DIR}/CMakeLists.txt"
  @ONLY)

add_subdirectory(imgui)

# Add SDL3 to the dependencies of imgui
cmake_policy(PUSH)
cmake_policy(SET CMP0079 NEW)
target_link_libraries(imgui PRIVATE SDL3::Headers)
cmake_policy(POP)

# Download and unpack spdlog at configure time
set(SPDLOG_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/spdlog-prefix")
set(SPDLOG_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/spdlog")
configure_file("cmake/spdlog-dl/CMakeLists.txt.in" "spdlog-dl/CMakeLists.txt")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/spdlog-dl")
execute_process(
  COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/spdlog-dl")

add_subdirectory(spdlog)
