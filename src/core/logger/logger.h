#pragma once

#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "loglevel.h"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/logger.h"

namespace tedlhy::minekraf::logger {

template<typename... Args>
using format_string_t = spdlog::format_string_t<Args...>;

struct Category {
  std::string name;
  LogLevel level;
};

using CategoryMap = std::map<unsigned, Category>;
using CategoryKeyT = CategoryMap::key_type;
using CategoryValT = CategoryMap::value_type;

static constexpr CategoryKeyT CATEGORY_NONE = 0;

struct LoggerInitParams {
  spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block;
  std::string filepath = "log.txt";
};

class Logger {
  LogLevel _level;
  CategoryMap categories;
  std::vector<spdlog::sink_ptr> sinks;
  std::map<std::string, std::shared_ptr<spdlog::logger>> loggers;
  std::mutex internalmutex;

  LoggerInitParams initparams;

  CategoryKeyT last_category = CATEGORY_NONE;  // for log() warning spam avoidance

  static spdlog::level::level_enum _to_spdlog_level(const LogLevel& level);

public:
  Logger(const LogLevel& level = LogLevel::info);
  ~Logger();

  /// Set logging level
  void level(const LogLevel& level);

  /// Get logging level
  const LogLevel& level() const;

  /**
   * Insert Category into categories map.
   *
   * This method returns true if insertion was successful, false otherwise.
   */
  bool insert_category(CategoryKeyT idx, Category&& category);

  /**
   * Remove Category from categories map.
   *
   * This method return true if removal was successful, false otherwise.
   */
  bool remove_category(CategoryKeyT idx);

  /**
   * Add logger sink.
   *
   * This method adds the provided sink to all initialized spdlog loggers.
   * You can also use it to preload sinks before calling init_loggers().
   *
   * Returns the size of sinks after the addition.
   */
  size_t add_sink(spdlog::sink_ptr sink);

  /**
   * Remove logger sink.
   *
   * This method removes the sink from all initialized spdlog loggers.
   *
   * Returns the size of sinks after removal.
   */
  size_t remove_sink(size_t idx);

  /**
   * Initialize SpdlogWriter loggers
   *
   * This method creates an async spdlog logger for every category known at the
   * time of calling this method. It also creates a default console out sink
   * and a file sink. By default, the async policy set is to block on full
   * message queue and the file sink is set to output to log.txt in the current
   * working directory.
   *
   * If this log writer is already initialized, this method does nothing.
   */
  void init_loggers(LoggerInitParams params);

  /**
   * Reset loggers
   *
   * This method removes every logger from this instance. You can use this
   * method to reinit the whole log writer by calling init_loggers() directly
   * after.
   */
  void reset_loggers();

  template<typename... Args>
  inline void log(const CategoryKeyT category, const LogLevel level, format_string_t<Args...> fmt, Args&&... args)
  {
    if (level < _level) {
      return;
    }

    auto spdloglevel = _to_spdlog_level(level);

    auto _category = categories.find(category);
    if (_category == categories.end()) {
      // this deliberately throws an exception when the default logger/category doesn't exist (because they should)
      auto& def_category = categories.at(CATEGORY_NONE);
      auto& def_logger = loggers.at(def_category.name);

      // check last warned category, eliminates most warning spam
      if (category != last_category) {
        def_logger->warn("Logging category with index {} does not exist", category);
        last_category = category;
      }

      // log the message with the default category
      def_logger->log(spdloglevel, fmt, std::forward<Args>(args)...);
      return;
    }

    auto _logger = loggers.find(_category->second.name);
    if (_logger == loggers.end()) {
      const std::lock_guard<std::mutex> lock(internalmutex);

      // category exists without a logger, create one in-place
      auto logger = std::make_shared<spdlog::async_logger>(_category->second.name, sinks.begin(), sinks.end(),
        spdlog::thread_pool(), initparams.policy);
      logger->set_level(_to_spdlog_level(_category->second.level));
      logger->trace("spdlog logger [{}] initialized", logger->name());
      logger->log(spdloglevel, fmt, std::forward<Args>(args)...);
      loggers[_category->second.name] = std::move(logger);
      return;
    }

    _logger->second->log(spdloglevel, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void log(const LogLevel level, format_string_t<Args...> fmt, Args&&... args)
  {
    log(CATEGORY_NONE, level, fmt, std::forward<Args>(args)...);
  }

  template<typename T>
  void log(const CategoryKeyT category, const LogLevel level, const T& msg)
  {
    log(category, level, "{}", msg);
  }

  template<typename T>
  void log(const LogLevel level, const T& msg)
  {
    log(CATEGORY_NONE, level, "{}", msg);
  }

  template<typename... Args>
  inline void trace(const CategoryKeyT category, format_string_t<Args...> fmt, Args&&... args)
  {
    log(category, LogLevel::trace, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void trace(format_string_t<Args...> fmt, Args&&... args)
  {
    log(CATEGORY_NONE, LogLevel::trace, fmt, std::forward<Args>(args)...);
  }

  template<typename T>
  void trace(const CategoryKeyT category, const T& msg)
  {
    log(category, LogLevel::trace, "{}", msg);
  }

  template<typename T>
  void trace(const T& msg)
  {
    log(CATEGORY_NONE, LogLevel::trace, "{}", msg);
  }

  template<typename... Args>
  inline void debug(const CategoryKeyT category, format_string_t<Args...> fmt, Args&&... args)
  {
    log(category, LogLevel::debug, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void debug(format_string_t<Args...> fmt, Args&&... args)
  {
    log(CATEGORY_NONE, LogLevel::debug, fmt, std::forward<Args>(args)...);
  }

  template<typename T>
  void debug(const CategoryKeyT category, const T& msg)
  {
    log(category, LogLevel::debug, "{}", msg);
  }

  template<typename T>
  void debug(const T& msg)
  {
    log(CATEGORY_NONE, LogLevel::debug, "{}", msg);
  }

  template<typename... Args>
  inline void info(const CategoryKeyT category, format_string_t<Args...> fmt, Args&&... args)
  {
    log(category, LogLevel::info, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void info(format_string_t<Args...> fmt, Args&&... args)
  {
    log(CATEGORY_NONE, LogLevel::info, fmt, std::forward<Args>(args)...);
  }

  template<typename T>
  void info(const CategoryKeyT category, const T& msg)
  {
    log(category, LogLevel::info, "{}", msg);
  }

  template<typename T>
  void info(const T& msg)
  {
    log(CATEGORY_NONE, LogLevel::info, "{}", msg);
  }

  template<typename... Args>
  inline void warning(const CategoryKeyT category, format_string_t<Args...> fmt, Args&&... args)
  {
    log(category, LogLevel::warning, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void warning(format_string_t<Args...> fmt, Args&&... args)
  {
    log(CATEGORY_NONE, LogLevel::warning, fmt, std::forward<Args>(args)...);
  }

  template<typename T>
  void warning(const CategoryKeyT category, const T& msg)
  {
    log(category, LogLevel::warning, "{}", msg);
  }

  template<typename T>
  void warning(const T& msg)
  {
    log(CATEGORY_NONE, LogLevel::warning, "{}", msg);
  }

  template<typename... Args>
  inline void error(const CategoryKeyT category, format_string_t<Args...> fmt, Args&&... args)
  {
    log(category, LogLevel::error, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void error(format_string_t<Args...> fmt, Args&&... args)
  {
    log(CATEGORY_NONE, LogLevel::error, fmt, std::forward<Args>(args)...);
  }

  template<typename T>
  void error(const CategoryKeyT category, const T& msg)
  {
    log(category, LogLevel::error, "{}", msg);
  }

  template<typename T>
  void error(const T& msg)
  {
    log(CATEGORY_NONE, LogLevel::error, "{}", msg);
  }

  template<typename... Args>
  inline void critical(const CategoryKeyT category, format_string_t<Args...> fmt, Args&&... args)
  {
    log(category, LogLevel::critical, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void critical(format_string_t<Args...> fmt, Args&&... args)
  {
    log(CATEGORY_NONE, LogLevel::critical, fmt, std::forward<Args>(args)...);
  }

  template<typename T>
  void critical(const CategoryKeyT category, const T& msg)
  {
    log(category, LogLevel::critical, "{}", msg);
  }

  template<typename T>
  void critical(const T& msg)
  {
    log(CATEGORY_NONE, LogLevel::critical, "{}", msg);
  }
};

/**
 * Get global logger instance
 */
Logger& global();

}  // namespace tedlhy::minekraf::logger
