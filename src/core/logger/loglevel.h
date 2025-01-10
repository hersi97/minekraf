#pragma once

#include "spdlog/fmt/ostr.h"

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

std::ostream& operator<<(std::ostream& os, const LogLevel& level);

}  // namespace tedlhy::minekraf::logger

template<>
struct fmt::formatter<tedlhy::minekraf::logger::LogLevel> : fmt::ostream_formatter {};
