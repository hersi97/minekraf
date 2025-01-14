#include "windowmanager_enums.h"

using namespace tedlhy::minekraf;

// WindowFullscreenMode methods

WindowFullscreenMode::WindowFullscreenMode(WindowFullscreenModeEnum mode) : fullscreenmode(mode)
{
}

WindowFullscreenMode& WindowFullscreenMode::operator=(WindowFullscreenModeEnum mode)
{
  fullscreenmode = mode;
  return *this;
}

bool WindowFullscreenMode::operator==(WindowFullscreenModeEnum mode) const
{
  return fullscreenmode == mode;
}

bool WindowFullscreenMode::operator==(const WindowFullscreenMode& mode) const
{
  return fullscreenmode == mode.fullscreenmode;
}

WindowFullscreenMode::operator std::string() const
{
  return fullscreenmode < _WindowFullscreenModeEnum_size ? WindowFullscreenModeString[fullscreenmode] : "N/A";
}

WindowFullscreenMode::operator const char*() const
{
  return fullscreenmode < _WindowFullscreenModeEnum_size ? WindowFullscreenModeString[fullscreenmode] : "N/A";
}

WindowFullscreenMode::operator WindowFullscreenModeEnum() const
{
  return fullscreenmode;
}

// WindowVSyncMode methods

WindowVSyncMode::WindowVSyncMode(WindowVSyncModeEnum mode) : vsyncmode(mode)
{
}

WindowVSyncMode& WindowVSyncMode::operator=(WindowVSyncModeEnum mode)
{
  vsyncmode = mode;
  return *this;
}

bool WindowVSyncMode::operator==(WindowVSyncModeEnum mode) const
{
  return vsyncmode == mode;
}

bool WindowVSyncMode::operator==(const WindowVSyncMode& mode) const
{
  return vsyncmode == mode.vsyncmode;
}

WindowVSyncMode::operator WindowVSyncModeEnum() const
{
  return vsyncmode;
}

WindowVSyncMode::operator std::string() const
{
  return vsyncmode < _WindowVSyncModeEnum_size ? WindowVSyncModeString[vsyncmode] : "N/A";
}

WindowVSyncMode::operator const char*() const
{
  return vsyncmode < _WindowVSyncModeEnum_size ? WindowVSyncModeString[vsyncmode] : "N/A";
}

// WindowAttributeResult methods

WindowAttributeResult::WindowAttributeResult(WindowAttributeResultEnum result) : attrresult(result)
{
}

WindowAttributeResult& WindowAttributeResult::operator=(WindowAttributeResultEnum result)
{
  attrresult = result;
  return *this;
}

bool WindowAttributeResult::operator==(WindowAttributeResultEnum result) const
{
  return attrresult == result;
}

bool WindowAttributeResult::operator==(const WindowAttributeResult& result) const
{
  return attrresult == result.attrresult;
}

WindowAttributeResult::operator WindowAttributeResultEnum() const
{
  return attrresult;
}

WindowAttributeResult::operator std::string() const
{
  return attrresult < _WindowAttributeResultEnum_size ? WindowAttributeResultString[attrresult] : "N/A";
}

WindowAttributeResult::operator const char*() const
{
  return attrresult < _WindowAttributeResultEnum_size ? WindowAttributeResultString[attrresult] : "N/A";
}

// namespace methods
namespace tedlhy::minekraf {

bool operator==(WindowFullscreenMode::WindowFullscreenModeEnum lhs, const WindowFullscreenMode& rhs)
{
  return rhs == lhs;
}

std::ostream& operator<<(std::ostream& os, const WindowFullscreenMode& mode)
{
  return os << static_cast<std::string>(mode);
}

bool operator==(WindowVSyncMode::WindowVSyncModeEnum lhs, const WindowVSyncMode& rhs)
{
  return rhs == lhs;
}

std::ostream& operator<<(std::ostream& os, const WindowVSyncMode& mode)
{
  return os << static_cast<std::string>(mode);
}

bool operator==(WindowAttributeResult::WindowAttributeResultEnum lhs, const WindowAttributeResult& rhs)
{
  return rhs == lhs;
}

std::ostream& operator<<(std::ostream& os, const WindowAttributeResult& result)
{
  return os << static_cast<std::string>(result);
}

}  // namespace tedlhy::minekraf
