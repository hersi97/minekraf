#pragma once

#include <memory>

#include "logger/logger.h"
#include "window/windowmanager.h"
#include "eventqueue.h"

namespace tedlhy::minekraf {

class App {
  bool running;

  std::unique_ptr<WindowManager> window_mgr;
  std::shared_ptr<logger::Logger> logger;

  EventQueue& eventqueue;

  App();

  void preUpdate(double deltatime);
  void update(double deltatime);
  void postUpdate(double deltatime);

public:
  static App& get();
  ~App();

  void run();
  void exit();
};

}  // namespace tedlhy::minekraf
