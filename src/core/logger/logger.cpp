#include "logger.h"

#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace tedlhy::minekraf::logger;

spdlog::level::level_enum Logger::_to_spdlog_level(const LogLevel& level)
{
  auto spdlevel = spdlog::level::off;
  switch (level) {
    case LogLevel::trace:
      spdlevel = spdlog::level::trace;
      break;
    case LogLevel::debug:
      spdlevel = spdlog::level::debug;
      break;
    case LogLevel::info:
      spdlevel = spdlog::level::info;
      break;
    case LogLevel::warning:
      spdlevel = spdlog::level::warn;
      break;
    case LogLevel::error:
      spdlevel = spdlog::level::err;
      break;
    case LogLevel::critical:
      spdlevel = spdlog::level::critical;
      break;
    default:
      break;
  }
  return spdlevel;
}

Logger::Logger(const LogLevel& level) : _level(level)
{
  categories[CATEGORY_NONE] = {"app", LogLevel::info};
}

Logger::~Logger()
{
}

void Logger::level(const LogLevel& level)
{
  trace("Setting logger level to: {}", level);
  _level = level;
}

const LogLevel& Logger::level() const
{
  return _level;
}

bool Logger::insert_category(CategoryKeyT idx, Category&& category)
{
  const std::lock_guard<std::mutex> lock(internalmutex);

  auto [_, success] = categories.emplace(idx, std::move(category));
  return success;
}

bool Logger::remove_category(CategoryKeyT idx)
{
  const std::lock_guard<std::mutex> lock(internalmutex);

  return categories.erase(idx);
}

size_t Logger::add_sink(spdlog::sink_ptr sink)
{
  const std::lock_guard<std::mutex> lock(internalmutex);

  sinks.push_back(std::move(sink));
  for (auto& [_, logger] : loggers) {
    logger->sinks().push_back(sinks.back());
  }
  return sinks.size();
}

size_t Logger::remove_sink(size_t idx)
{
  const std::lock_guard<std::mutex> lock(internalmutex);

  for (auto& [_, logger] : loggers) {
    auto& loggersinks = logger->sinks();
    loggersinks.erase(loggersinks.begin() + idx);
  }
  sinks.erase(sinks.begin() + idx);
  return sinks.size();
}

void Logger::init_loggers(LoggerInitParams params)
{
  const std::lock_guard<std::mutex> lock(internalmutex);

  if (!loggers.empty()) {
    return;
  }

  // Set init params
  initparams = std::move(params);

  // Create default sinks
  if (sinks.size() < 1) {
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
  }
  if (sinks.size() < 2) {
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(initparams.filepath));
  }

  // Create loggers for categories
  for (const auto& [_, category] : categories) {
    auto logger = std::make_shared<spdlog::async_logger>(category.name, sinks.begin(), sinks.end(),
      spdlog::thread_pool(), initparams.policy);
    logger->set_level(_to_spdlog_level(category.level));
    logger->trace("spdlog logger [{}] initialized", logger->name());
    loggers[category.name] = std::move(logger);
  }
}

void Logger::reset_loggers()
{
  const std::lock_guard<std::mutex> lock(internalmutex);

  for (const auto& [_, logger] : loggers) {
    logger->trace("resetting spdlog logger [{}]...", logger->name());
  }
  loggers.clear();
}

Logger& global()
{
  static Logger _global_logger;

  return _global_logger;
}
