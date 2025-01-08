#pragma once

#include <format>

namespace tedlhy::minekraf::logger {
struct LogLevel {
  enum LogLevelEnum {
    trace,
    debug,
    info,
    warning,
    error,
    critical,
    _LogLevel_size,
  };

private:
  static constexpr const char* LogLevelString[] = {
    "trace",
    "debug",
    "info",
    "warning",
    "error",
    "critical",
  };

  LogLevelEnum level;

public:
  LogLevel(LogLevelEnum level);
  LogLevel& operator=(LogLevelEnum level);
  bool operator==(LogLevelEnum level) const;
  bool operator==(const LogLevel& level) const;
  bool operator<(LogLevelEnum level) const;
  bool operator<(const LogLevel& level) const;
  bool operator>(LogLevelEnum level) const;
  bool operator>(const LogLevel& level) const;
  operator LogLevelEnum() const;
  operator std::string() const;
};

bool operator==(LogLevel::LogLevelEnum lhs, const LogLevel& rhs);
bool operator<(LogLevel::LogLevelEnum lhs, const LogLevel& rhs);
bool operator>(LogLevel::LogLevelEnum lhs, const LogLevel& rhs);

}  // namespace tedlhy::minekraf::logger

namespace std {
using namespace tedlhy::minekraf::logger;

template<>
struct formatter<LogLevel> : formatter<std::string> {
  auto format(const LogLevel& level, format_context& ctx) const
  {
    return formatter<std::string>::format(level, ctx);
  }
};
}  // namespace std
