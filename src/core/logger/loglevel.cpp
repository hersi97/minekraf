#include "loglevel.h"

using namespace tedlhy::minekraf::logger;

LogLevel::LogLevel(LogLevelEnum level)
{
  this->level = level;
}

LogLevel& LogLevel::operator=(LogLevelEnum level)
{
  this->level = level;
  return *this;
}

bool LogLevel::operator==(LogLevelEnum level) const
{
  return this->level == level;
}

bool LogLevel::operator==(const LogLevel& level) const
{
  return this->level == level.level;
}

bool LogLevel::operator<(LogLevelEnum level) const
{
  return this->level < level;
}

bool LogLevel::operator<(const LogLevel& level) const
{
  return this->level < level.level;
}

bool LogLevel::operator>(LogLevelEnum level) const
{
  return this->level > level;
}

bool LogLevel::operator>(const LogLevel& level) const
{
  return this->level > level.level;
}

LogLevel::operator LogLevelEnum() const
{
  return level;
}

LogLevel::operator std::string() const
{
  return level < _LogLevel_size ? LogLevelString[level] : "N/A";
}

namespace tedlhy::minekraf::logger {

bool operator==(LogLevel::LogLevelEnum lhs, const LogLevel& rhs)
{
  return rhs == lhs;
}

bool operator<(LogLevel::LogLevelEnum lhs, const LogLevel& rhs)
{
  return rhs > lhs;
}

bool operator>(LogLevel::LogLevelEnum lhs, const LogLevel& rhs)
{
  return rhs < lhs;
}

std::ostream& operator<<(std::ostream& os, const LogLevel& level)
{
  return os << static_cast<std::string>(level);
}

}  // namespace tedlhy::minekraf::logger
