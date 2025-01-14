#pragma once

#include <tuple>

#include "SDL3/SDL.h"

#include "core/window/windowmanager.h"

namespace tedlhy::minekraf {

struct WindowManager::Impl {
  SDL_Window *window;
  SDL_GLContext context;

public:
  Impl();
  ~Impl();

  bool icon(SDL_Surface *icon);

  bool sync();

  bool swap();
};

};  // namespace tedlhy::minekraf
