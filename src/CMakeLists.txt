target_sources(minekraf
  PRIVATE "${CMAKE_CURRENT_LIST_DIR}/main.cpp")

target_include_directories(minekraf
  PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

configure_file("version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h")

add_subdirectory(core)
