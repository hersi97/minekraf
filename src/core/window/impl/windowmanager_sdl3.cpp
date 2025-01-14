#include "windowmanager_sdl3.h"

#include <vector>
#include <stdexcept>

#include "SDL3/SDL.h"

#include "core/eventqueue.h"

using namespace tedlhy::minekraf;

SDL_DisplayMode *find_closest_displaymode(SDL_Window *window, int width, int height)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Get display id for window");
  SDL_DisplayID displayid = SDL_GetDisplayForWindow(window);
  if (!displayid) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to get display id for window (%s)", SDL_GetError());
    return nullptr;
  }

  int displaymodes_count;
  SDL_DisplayMode **displaymodes = SDL_GetFullscreenDisplayModes(displayid, &displaymodes_count);

  for (int i = 0; i < displaymodes_count; i++) {
    SDL_DisplayMode *displaymode = displaymodes[i];
    if (displaymode->w > width) {
      // too wide
      continue;
    }
    if (displaymode->h > height) {
      // too high
      continue;
    }

    // found a suitable mode
    SDL_free(displaymodes);

    SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Get pixel format for displaymode[%d]", i);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(displaymode->format);
    if (!format) {
      SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Could not get pixel format details for %s",
        SDL_GetPixelFormatName(displaymode->format));
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Found suitable fullscreen displaymode (%dx%dx%dbits %.3fHz)", displaymode->w,
      displaymode->h, format ? format->bits_per_pixel : 0, displaymode->refresh_rate);
    return displaymode;
  }

  SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Did not find a suitable fullscreen displaymode");
  SDL_free(displaymodes);
  return nullptr;
}

// Impl methods

WindowManager::Impl::Impl() : window(nullptr), context(nullptr)
{
}

WindowManager::Impl::~Impl()
{
  if (context) {
    SDL_GL_DestroyContext(context);
    context = nullptr;
  }

  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
}

bool WindowManager::Impl::icon(SDL_Surface *icon)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window icon: %p", static_cast<void *>(icon));
  return SDL_SetWindowIcon(window, icon);
}

bool WindowManager::Impl::sync()
{
  return SDL_SyncWindow(window);
}

bool WindowManager::Impl::swap()
{
  return SDL_GL_SwapWindow(window);
}

// WindowManager methods

WindowManager::WindowManager(WindowManagerInitParams params) : _impl(new Impl())
{
  if (!SDL_IsMainThread()) {
    throw std::runtime_error("SDL3 WindowManager must be initialized on the main thread");
  }

  if (SDL_WasInit(SDL_INIT_VIDEO) != SDL_INIT_VIDEO) {
    throw std::runtime_error("SDL3 Video Subsystem was not initialized.");
  }

  SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
  if (params.resizable) {
    flags |= SDL_WINDOW_RESIZABLE;
  }
  if (params.fullscreen) {
    flags |= SDL_WINDOW_FULLSCREEN;
  }

  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Creating window: %s (%dx%d)", params.title, params.width, params.height);
  _impl->window = SDL_CreateWindow(params.title, params.width, params.height, flags);
  if (!_impl->window) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create SDL window (%s)", SDL_GetError());
    throw std::runtime_error("Failed to create SDL window");
  }

  if (!params.fullscreen) {
    int x = params.initX, y = params.initY;
    if (params.initX < 0) {
      x = SDL_WINDOWPOS_UNDEFINED;
    }
    if (params.initY < 0) {
      y = SDL_WINDOWPOS_UNDEFINED;
    }

    SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window position: { x: %d, y: %d }", params.initX, params.initY);
    SDL_SetWindowPosition(_impl->window, x, y);
  }

  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window fullscreen: %s", params.fullscreen ? "on" : "off");
  SDL_SetWindowFullscreen(_impl->window, params.fullscreen);
  SDL_DisplayMode *displaymode = nullptr;
  switch (params.fullscreenMode) {
    case WindowFullscreenMode::borderless:
      displaymode = nullptr;
      break;
    case WindowFullscreenMode::exclusive:
      displaymode = find_closest_displaymode(_impl->window, params.width, params.height);
      break;
    default:
      break;
  };
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set fullscreen mode: %s", static_cast<const char *>(params.fullscreenMode));
  if (!SDL_SetWindowFullscreenMode(_impl->window, displaymode)) {
    SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Unsupported fullscreen mode: %s (%s)",
      static_cast<const char *>(params.fullscreenMode), SDL_GetError());
  }

  SDL_LogTrace(SDL_LOG_CATEGORY_INPUT, "Set mouse grab mode: %s", params.mouseGrab ? "on" : "off");
  if (!SDL_SetWindowMouseGrab(_impl->window, params.mouseGrab)) {
    SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Failed to grab mouse (%s)", SDL_GetError());
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Creating OpenGL context");
  _impl->context = SDL_GL_CreateContext(_impl->window);
  if (!_impl->context) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create OpenGL context (%s)", SDL_GetError());
    throw std::runtime_error("Failed to create window GL context");
  }

  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Setting current OpenGL context to window");
  if (!SDL_GL_MakeCurrent(_impl->window, _impl->context)) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to set OpenGL context to window (%s)", SDL_GetError());
    throw std::runtime_error("Failed to set OpenGL context to window");
  }

  int vsync_mode = SDL_WINDOW_SURFACE_VSYNC_DISABLED;
  switch (params.vsyncMode) {
    case WindowVSyncMode::disabled:
      vsync_mode = SDL_WINDOW_SURFACE_VSYNC_DISABLED;
      break;
    case WindowVSyncMode::enabled:
      vsync_mode = 1;
      break;
    case WindowVSyncMode::adaptive:
      vsync_mode = SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE;
      break;
    default:
      break;
  }
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set VSync mode: %s", static_cast<const char *>(params.vsyncMode));
  if (!SDL_GL_SetSwapInterval(vsync_mode)) {
    SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Unsupported VSync mode: %s (%s)", static_cast<const char *>(params.vsyncMode),
      SDL_GetError());
  }
}

WindowManager::~WindowManager()
{
}

void WindowManager::preUpdate(float deltatime)
{
  // Handle Display and Window events
  std::vector<SDL_Event> events;
  int numevents = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_EVENT_DISPLAY_FIRST, SDL_EVENT_DISPLAY_LAST);
  if (numevents > 0) {
    events.resize(numevents);
    SDL_PeepEvents(events.data(), numevents, SDL_GETEVENT, SDL_EVENT_DISPLAY_FIRST, SDL_EVENT_DISPLAY_LAST);
  }
  numevents = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_EVENT_WINDOW_FIRST, SDL_EVENT_WINDOW_LAST);
  if (numevents > 0) {
    size_t events_sz = events.size();
    events.resize(events.size() + numevents);
    SDL_PeepEvents(events.data() + events_sz, numevents, SDL_GETEVENT, SDL_EVENT_WINDOW_FIRST, SDL_EVENT_WINDOW_LAST);
  }

  // TODO: EventQueue
  auto &eventqueue = EventQueue::get();
  /*
  for (auto &event : events) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    // TODO: handle window events
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
      SDL_PushEvent(&event);
    }
  }
  */
}

void WindowManager::update(float deltatime)
{
}

void WindowManager::postUpdate(float deltatime)
{
  _impl->swap();
}

bool WindowManager::title(const char *title)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window title: %s", title);
  return SDL_SetWindowTitle(_impl->window, title);
}

const char *WindowManager::title()
{
  return SDL_GetWindowTitle(_impl->window);
}

bool WindowManager::size(int width, int height)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window size: %dx%d", width, height);
  return SDL_SetWindowSize(_impl->window, width, height);
}

std::tuple<int, int> WindowManager::size()
{
  int w, h;
  SDL_GetWindowSize(_impl->window, &w, &h);
  return {w, h};
}

std::tuple<int, int> WindowManager::sizeInPx()
{
  int w, h;
  SDL_GetWindowSizeInPixels(_impl->window, &w, &h);
  return {w, h};
}

bool WindowManager::position(int x, int y)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window position: { x: %d, y: %d }", x, y);
  return SDL_SetWindowPosition(_impl->window, x, y);
}

std::tuple<int, int> WindowManager::position()
{
  int x, y;
  SDL_GetWindowPosition(_impl->window, &x, &y);
  return {x, y};
}

bool WindowManager::resizable(bool resizable)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window resizable: %s", resizable ? "on" : "off");
  return SDL_SetWindowResizable(_impl->window, resizable);
}

bool WindowManager::resizable()
{
  return SDL_GetWindowFlags(_impl->window) & SDL_WINDOW_RESIZABLE;
}

bool WindowManager::fullscreen(bool fullscreen)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set window fullscreen: %s", fullscreen ? "on" : "off");
  return SDL_SetWindowFullscreen(_impl->window, fullscreen);
}

bool WindowManager::fullscreen()
{
  return SDL_GetWindowFlags(_impl->window) & SDL_WINDOW_FULLSCREEN;
}

bool WindowManager::fullscreenMode(WindowFullscreenMode mode)
{
  SDL_DisplayMode *displaymode = nullptr;
  switch (mode) {
    case WindowFullscreenMode::borderless: {
      displaymode = nullptr;
      break;
    }
    case WindowFullscreenMode::exclusive: {
      auto [width, height] = sizeInPx();
      displaymode = find_closest_displaymode(_impl->window, width, height);
      break;
    }
    default: {
      break;
    }
  };
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set fullscreen mode: %s", static_cast<const char *>(mode));
  if (!SDL_SetWindowFullscreenMode(_impl->window, displaymode)) {
    SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Unsupported fullscreen mode: %s (%s)", static_cast<const char *>(mode),
      SDL_GetError());
    return false;
  }
  return true;
}

WindowFullscreenMode WindowManager::fullscreenMode()
{
  if (SDL_GetWindowFullscreenMode(_impl->window)) {
    return WindowFullscreenMode::exclusive;
  }
  return WindowFullscreenMode::borderless;
}

bool WindowManager::vsyncMode(WindowVSyncMode mode)
{
  int vsync_mode = SDL_WINDOW_SURFACE_VSYNC_DISABLED;
  switch (mode) {
    case WindowVSyncMode::disabled:
      vsync_mode = SDL_WINDOW_SURFACE_VSYNC_DISABLED;
      break;
    case WindowVSyncMode::enabled:
      vsync_mode = 1;
      break;
    case WindowVSyncMode::adaptive:
      vsync_mode = SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE;
      break;
    default:
      break;
  }
  SDL_LogTrace(SDL_LOG_CATEGORY_VIDEO, "Set VSync mode: %s", static_cast<const char *>(mode));
  if (!SDL_GL_SetSwapInterval(vsync_mode)) {
    SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Unsupported VSync mode: %s (%s)", static_cast<const char *>(mode),
      SDL_GetError());
    return false;
  }
  return true;
}

WindowVSyncMode WindowManager::vsyncMode()
{
  int vsync_mode = SDL_WINDOW_SURFACE_VSYNC_DISABLED;
  if (!SDL_GL_GetSwapInterval(&vsync_mode)) {
    SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Could not query VSync mode (%s)", SDL_GetError());
    return WindowVSyncMode::disabled;
  }
  switch (vsync_mode) {
    case SDL_WINDOW_SURFACE_VSYNC_DISABLED:
      return WindowVSyncMode::disabled;
    case SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE:
      return WindowVSyncMode::adaptive;
    default:
      return WindowVSyncMode::enabled;
  }
}

bool WindowManager::mouseGrab(bool grab)
{
  SDL_LogTrace(SDL_LOG_CATEGORY_INPUT, "Set mouse grab mode: %s", grab ? "on" : "off");
  return SDL_SetWindowMouseGrab(_impl->window, grab);
}

bool WindowManager::mouseGrab()
{
  return SDL_GetWindowMouseGrab(_impl->window);
}

bool WindowManager::show()
{
  return SDL_ShowWindow(_impl->window);
}

bool WindowManager::hide()
{
  return SDL_HideWindow(_impl->window);
}

bool WindowManager::maximize()
{
  return SDL_MaximizeWindow(_impl->window);
}

bool WindowManager::minimize()
{
  return SDL_MinimizeWindow(_impl->window);
}

bool WindowManager::restore()
{
  return SDL_RestoreWindow(_impl->window);
}

bool WindowManager::raise()
{
  return SDL_RaiseWindow(_impl->window);
}
