#include "app.h"

#include <vector>
#include <numeric>

#include "SDL3/SDL.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_opengl.h"

#include "imgui_impl_sdl3.h"

#include "version.h"

using namespace tedlhy::minekraf;

static logger::LogLevel _sdl_log_prio_to_lvl(SDL_LogPriority priority)
{
  logger::LogLevel level = logger::LogLevel::info;
  switch (priority) {
    case SDL_LOG_PRIORITY_INVALID:
      break;
    case SDL_LOG_PRIORITY_TRACE:
      level = logger::LogLevel::trace;
      break;
    case SDL_LOG_PRIORITY_VERBOSE:
    case SDL_LOG_PRIORITY_DEBUG:
      level = logger::LogLevel::debug;
      break;
    case SDL_LOG_PRIORITY_INFO:
      level = logger::LogLevel::info;
      break;
    case SDL_LOG_PRIORITY_WARN:
      level = logger::LogLevel::warning;
      break;
    case SDL_LOG_PRIORITY_ERROR:
      level = logger::LogLevel::error;
      break;
    case SDL_LOG_PRIORITY_CRITICAL:
      level = logger::LogLevel::critical;
      break;
    default:
      break;
  }
  return level;
}

static logger::LogLevel _sdl_log_cat_to_lvl(SDL_LogCategory category)
{
  return _sdl_log_prio_to_lvl(SDL_GetLogPriority(category));
}

void App::preUpdate(double deltatime)
{
  window_mgr->preUpdate(deltatime);
  eventqueue.tick();
  SDL_PumpEvents();  // force event queue udate for SDL, since we are filtering
}

void App::update(double deltatime)
{
  window_mgr->update(deltatime);
  // clear screen with dark magenta
  glClearColor(0.2, 0.05, 0.2, 1);
  glClear(GL_COLOR_BUFFER_BIT);
}

void App::postUpdate(double deltatime)
{
  window_mgr->postUpdate(deltatime);
}

App::App() : running(false), window_mgr(), logger(), eventqueue(EventQueue::get())
{
  std::atexit(SDL_Quit);  // register SDL_Quit on application exit

  SDL_SetAppMetadata("Minekraf", versionstr, "tedlhy.minekraf");

  // Init default logger

  logger = std::make_shared<logger::Logger>(logger::LoggerInitParams{
    .defaultlevel = logger::LogLevel::trace,
    .filepath = "log.txt",
  });
  logger::set(logger);

  // Init SDL

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    logger->error("Error initializing SDL3 ({})", SDL_GetError());
    throw std::runtime_error("Error initializing SDL3");
  }

  // Init event queue

  auto sdl_app_exit_handler = [](EventID id, void* data, void* categorydata) -> int {
    App* app = static_cast<App*>(categorydata);
    SDL_Event* eventdata = static_cast<SDL_Event*>(data);

    auto logger = logger::get();
    logger->trace("sdl_app_exit_handler(): {{ event: {:#x}, &app: {:p}, &event->type: {:#x} }}", id, (void*)app,
      eventdata->type);

    if (!app) {
      // something went horribly wrong
      logger->critical("Failure to cast App data!");
      throw std::runtime_error("Failure to cast App data");
    }

    if (!eventdata) {
      // something went horribly wrong
      logger->critical("Failure to cast SDL_Event data!");
      throw std::runtime_error("Failure to cast SDL_Event data");
    }

    switch (id) {
      case SDL_EVENT_QUIT:
      case SDL_EVENT_TERMINATING:
        break;
      default:
        logger->critical("{} is not a valid exit event id", id);
        throw std::runtime_error("exit event handler called with invalid id");
    }
    app->exit();
    return 0;
  };

  EventCategoryID id = eventqueue.find_next_free_category(0);
  eventqueue.insert_category(
    EventCategory{
      .id = id,
      .name = "SDL_ApplicationExit",
      .event_ids = {SDL_EVENT_QUIT, SDL_EVENT_TERMINATING},
      .handlers = {sdl_app_exit_handler},
    },
    this, 0);

  // add EventQueue callback (SDL_EventFilter)
  auto callback_SDL_Event = [](void* userdata, SDL_Event* event) {
    EventQueue* eventqueue = static_cast<EventQueue*>(userdata);

    auto logger = logger::get();

    if (!eventqueue) {
      // something went horribly wrong
      logger->critical("Failure to cast EventQueue data!");
      throw std::runtime_error("Failure to cast EventQueue data");
    }

    // copy event and store in event queue
    logger->trace("callback_SDL_Event(): pushing event {:#x}", event->type);
    eventqueue->push_event(event->type, &event, sizeof(SDL_Event));
    return event->type == SDL_EVENT_QUIT;
  };

  SDL_SetEventFilter(callback_SDL_Event, &eventqueue);

  // Init SDL Logger

  auto logger_LogOutputFunction = [](void* userdata, int category, SDL_LogPriority priority, const char* message) {
    if (!userdata) {
      return;
    }

    logger::Logger* logger = static_cast<logger::Logger*>(userdata);
    auto level = _sdl_log_prio_to_lvl(priority);
    logger->log(static_cast<logger::CategoryKeyT>(category), level, message);
  };

  // WARN: DELETE THESE AFTER DEBUGGING
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_TRACE);

  // Add SDL logging categories to logger
  std::map<SDL_LogCategory, std::string> names{
    {SDL_LOG_CATEGORY_APPLICATION, "app"},
    {SDL_LOG_CATEGORY_ERROR, "error"},
    {SDL_LOG_CATEGORY_ASSERT, "assert"},
    {SDL_LOG_CATEGORY_SYSTEM, "system"},
    {SDL_LOG_CATEGORY_AUDIO, "audio"},
    {SDL_LOG_CATEGORY_VIDEO, "video"},
    {SDL_LOG_CATEGORY_RENDER, "render"},
    {SDL_LOG_CATEGORY_INPUT, "input"},
    {SDL_LOG_CATEGORY_TEST, "test"},
    {SDL_LOG_CATEGORY_GPU, "gpu"},
  };
  for (const auto& [category, name] : names) {
    if (!logger->insert_category(category, {name, _sdl_log_cat_to_lvl(category)})) {
      // maybe we already have a category like that, try setting category level
      logger->set_category_level(category, _sdl_log_cat_to_lvl(category));
    }
  }

  SDL_SetLogOutputFunction(logger_LogOutputFunction, logger.get());

  // TODO: Add ImGui logger sink

  logger->info("Yippie!");

  WindowManagerInitParams params{
    .title = "Minekraf",
    .width = 1280,
    .height = 720,
  };
  window_mgr = std::make_unique<WindowManager>(params);

  // TODO: Add gui manager

  window_mgr->show();
}

App& App::get()
{
  static App instance;

  return instance;
}

App::~App()
{
  logger->trace("App::~App()");
}

void App::run()
{
  using namespace std::chrono;
  {
    const auto steady_clock_period = steady_clock::duration{1};
    const auto millisecond = milliseconds{1};
    if (steady_clock_period > millisecond) {
      logger->warning(
        "steady_clock has a clock period of {}, which is longer than the recommended {}, "
        "frame timing might be imprecise",
        steady_clock_period, millisecond);
    }
  }

  running = true;

  auto last_time_point = steady_clock::now();
  duration<double> time_delta{milliseconds{16}};
  while (running) {
    preUpdate(time_delta.count());
    update(time_delta.count());
    postUpdate(time_delta.count());

    auto time_point = steady_clock::now();
    time_delta = duration<double>{time_point - last_time_point};
    std::swap(time_point, last_time_point);
  }
}

void App::exit()
{
  running = false;
}
