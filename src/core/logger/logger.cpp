#include "logger.h"

#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace tedlhy::minekraf::logger;

spdlog::level::level_enum Logger::to_spdlog_level(const LogLevel& level)
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

std::optional<Logger::LoggerMap::iterator> Logger::create_logger_from_category(const Category& category)
{
  auto logger = std::make_shared<spdlog::async_logger>(category.name, sinks.begin(), sinks.end(), spdlog::thread_pool(),
    initparams.policy);
  logger->set_level(to_spdlog_level(category.level));
  logger->trace("spdlog logger [{}] initialized", logger->name());
  auto [element, success] = loggers.emplace(category.name, std::move(logger));
  if (success) {
    return element;
  }
  // try to log warning about failure
  if (auto _def_cat = this->categories.find(CATEGORY_NONE); _def_cat != this->categories.end()) {
    if (auto _def_logger = loggers.find(_def_cat->second.name); _def_logger != loggers.end()) {
      _def_logger->second->warn("Could not create logger {}", category.name);
    }
  }
  return {};
}

Logger::Logger(LoggerInitParams params, std::initializer_list<CategoryMap::value_type> categories,
  std::initializer_list<std::shared_ptr<spdlog::sinks::sink>> sinks) :
  categories(categories), sinks(sinks), loggers(), _level(params.defaultlevel), initparams(std::move(params)), mutex()
{
  // Try create default category
  this->categories.emplace(CATEGORY_NONE, Category{"app", initparams.defaultlevel});

  // Create default sinks
  if (this->sinks.empty()) {
    this->sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    this->sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(initparams.filepath));
  }

  // init thread pool if it isn't already
  if (!spdlog::thread_pool()) {
    spdlog::init_thread_pool(8192, 1);
  }

  // Create loggers for categories
  for (const auto& [_, category] : this->categories) {
    create_logger_from_category(category);
  }
}

Logger::~Logger()
{
}

void Logger::level(const LogLevel& level)
{
  const std::lock_guard lock(mutex);
  trace("Setting logger level to: {}", level);
  _level = level;
}

const LogLevel& Logger::level() const
{
  return _level;
}

bool Logger::insert_category(CategoryKeyT idx, Category&& category)
{
  const std::lock_guard lock(mutex);

  auto [_category, success] = categories.emplace(idx, std::move(category));
  if (success) {
    // create logger for the category
    create_logger_from_category(_category->second);
  }
  return success;
}

bool Logger::remove_category(CategoryKeyT idx)
{
  const std::lock_guard lock(mutex);

  if (idx == CATEGORY_NONE) {
    // do not let default logger be deleted
    return false;
  }

  auto _category = categories.find(idx);
  if (_category == categories.end()) {
    return false;
  }

  // try to remove logger as well
  if (auto _logger = loggers.find(_category->second.name); _logger != loggers.end()) {
    _logger->second->debug("removing spdlog logger [{}]", _logger->first);
    loggers.erase(_logger);
  }

  return categories.erase(idx);
}

bool Logger::set_category_level(CategoryKeyT idx, const LogLevel& level)
{
  const std::lock_guard lock(mutex);

  auto _category = categories.find(idx);
  if (_category == categories.end()) {
    return false;
  }

  auto& category = _category->second;
  category.level = level;
  loggers.at(category.name)->set_level(to_spdlog_level(level));

  return true;
}

size_t Logger::add_sink(spdlog::sink_ptr sink)
{
  const std::lock_guard lock(mutex);

  sinks.push_back(std::move(sink));
  for (auto& [_, logger] : loggers) {
    logger->sinks().push_back(sinks.back());
  }
  return sinks.size();
}

size_t Logger::remove_sink(size_t idx)
{
  const std::lock_guard lock(mutex);

  if (idx >= sinks.size()) {
    return sinks.size();
  }

  for (auto& [name, logger] : loggers) {
    auto& loggersinks = logger->sinks();
    if (idx >= loggersinks.size()) {
      throw std::runtime_error(
        fmt::format("tried removing logger sink index {}, but spdlog logger [{}] sinks container has size {}", idx,
          name, loggersinks.size()));
    }
    loggersinks.erase(loggersinks.begin() + idx);
  }
  sinks.erase(sinks.begin() + idx);
  return sinks.size();
}

namespace tedlhy::minekraf::logger {

static std::unique_ptr<Logger> _global;

Logger& init_default(LoggerInitParams param)
{
  if (!_global) {
    _global = std::make_unique<Logger>(param);
  }
  return *_global;
}

Logger& get()
{
  assert(_global == nullptr && "Default logger is not initialized! Did you forget to call init_default()?");

  return *_global;
}
}  // namespace tedlhy::minekraf::logger
