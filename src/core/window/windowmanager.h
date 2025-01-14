#pragma once

#include <memory>
#include <tuple>

#include "windowmanager_enums.h"

namespace tedlhy::minekraf {

struct WindowManagerInitParams {
  const char *title = "MyApp";
  int width = 800;
  int height = 600;
  int initX = -1;  // -1: don't care
  int initY = -1;  // -1: don't care
  bool resizable = true;
  bool fullscreen = false;
  WindowFullscreenMode fullscreenMode = WindowFullscreenMode::borderless;
  WindowVSyncMode vsyncMode = WindowVSyncMode::enabled;
  bool mouseGrab = false;
};

class WindowManager {
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  WindowManager() = delete;
  ~WindowManager();

  WindowManager(WindowManagerInitParams params);
  void init(WindowManagerInitParams params = {});

  void preUpdate(float deltatime);
  void update(float deltatime);
  void postUpdate(float deltatime);

  bool title(const char *title);
  const char *title();

  bool size(int width, int height);
  std::tuple<int, int> size();
  std::tuple<int, int> sizeInPx();

  bool position(int x, int y);
  std::tuple<int, int> position();

  bool resizable(bool resizable);
  bool resizable();

  bool fullscreen(bool fullscreen);
  bool fullscreen();

  bool fullscreenMode(WindowFullscreenMode mode);
  WindowFullscreenMode fullscreenMode();

  bool vsyncMode(WindowVSyncMode mode);
  WindowVSyncMode vsyncMode();

  bool mouseGrab(bool grab);
  bool mouseGrab();

  bool show();
  bool hide();

  bool maximize();
  bool minimize();
  bool restore();
  bool raise();
};

}  // namespace tedlhy::minekraf
