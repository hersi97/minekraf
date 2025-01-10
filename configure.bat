@echo off

rem set generator to your current visual studio version here (check `cmake -G` for a list)
set generator="Visual Studio 17 2022"

set srcdir="%~dp0"
set bindir="%~dp0\build"

if not exist %bindir% (
  mkdir %bindir% || goto :error
)

cmake -G %generator% %* -S %srcdir% -B %bindir% || goto :cmakeerror

echo "Successfully configured CMake project"
exit /b 0

:error
  echo "Failed to create build directory"
  exit /b 1

:cmakeerror
  echo "Failed to configure CMake project"
  exit /b 2
