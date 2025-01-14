target_sources(minekraf PRIVATE
  windowmanager_enums.cpp
)

# windowmanager implementation
target_sources(minekraf PRIVATE
  impl/windowmanager_sdl3.cpp)
