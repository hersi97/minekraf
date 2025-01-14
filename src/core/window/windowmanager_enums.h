#pragma once

#include "spdlog/fmt/ostr.h"

namespace tedlhy::minekraf {

struct WindowFullscreenMode {
  enum WindowFullscreenModeEnum {
    borderless,
    exclusive,
    _WindowFullscreenModeEnum_size,
  };

private:
  static constexpr const char* WindowFullscreenModeString[] = {
    "Borderless Windowed",
    "Exclusive Fullscreen",
  };

  WindowFullscreenModeEnum fullscreenmode;

public:
  WindowFullscreenMode(WindowFullscreenModeEnum mode);
  WindowFullscreenMode& operator=(WindowFullscreenModeEnum mode);
  bool operator==(WindowFullscreenModeEnum mode) const;
  bool operator==(const WindowFullscreenMode& mode) const;
  operator WindowFullscreenModeEnum() const;
  operator std::string() const;
  operator const char*() const;
};

bool operator==(WindowFullscreenMode::WindowFullscreenModeEnum lhs, const WindowFullscreenMode& rhs);
std::ostream& operator<<(std::ostream& os, const WindowFullscreenMode& mode);

struct WindowVSyncMode {
  enum WindowVSyncModeEnum {
    disabled,
    enabled,
    adaptive,
    _WindowVSyncModeEnum_size,
  };

private:
  static constexpr const char* WindowVSyncModeString[] = {
    "VSync Disabled",
    "VSync Enabled",
    "VSync Adaptive",
  };

  WindowVSyncModeEnum vsyncmode;

public:
  WindowVSyncMode(WindowVSyncModeEnum mode);
  WindowVSyncMode& operator=(WindowVSyncModeEnum mode);
  bool operator==(WindowVSyncModeEnum mode) const;
  bool operator==(const WindowVSyncMode& mode) const;
  operator WindowVSyncModeEnum() const;
  operator std::string() const;
  operator const char*() const;
};

bool operator==(WindowVSyncMode::WindowVSyncModeEnum lhs, const WindowVSyncMode& rhs);
std::ostream& operator<<(std::ostream& os, const WindowVSyncMode& mode);

struct WindowAttributeResult {
  enum WindowAttributeResultEnum {
    success,
    failure,
    not_implemented,
    _WindowAttributeResultEnum_size,
  };

private:
  static constexpr const char* WindowAttributeResultString[] = {
    "Success",
    "Failure",
    "Not Implemented",
  };

  WindowAttributeResultEnum attrresult;

public:
  WindowAttributeResult(WindowAttributeResultEnum result);
  WindowAttributeResult& operator=(WindowAttributeResultEnum result);
  bool operator==(WindowAttributeResultEnum result) const;
  bool operator==(const WindowAttributeResult& result) const;
  operator WindowAttributeResultEnum() const;
  operator std::string() const;
  operator const char*() const;
};

bool operator==(WindowAttributeResult::WindowAttributeResultEnum lhs, const WindowAttributeResult& rhs);
std::ostream& operator<<(std::ostream& os, const WindowAttributeResult& result);

};  // namespace tedlhy::minekraf

namespace fmt {
using namespace tedlhy::minekraf;

template<>
struct formatter<WindowFullscreenMode> : ostream_formatter {};

template<>
struct formatter<WindowVSyncMode> : ostream_formatter {};

template<>
struct formatter<WindowAttributeResult> : ostream_formatter {};

}  // namespace fmt
