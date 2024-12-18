@echo off

set generator="Visual Studio 17 2022"
set distdir="%~dp0\dist"

mkdir build && cd build || goto :error

cmake -G %generator% -DCMAKE_INSTALL_PREFIX=%distdir% .. || goto :cmakeerror

echo "Successfully configured CMake project"
exit /b 0

:error
  echo "Failed to create build directory"
  exit /b 1

:cmakeerror
  echo "Failed to configure CMake project"
  exit /b 2
