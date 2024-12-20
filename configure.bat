@echo off

rem set generator to your current visual studio version here (check `cmake -G` for a list)
set generator="Visual Studio 17 2022"
set distdir="%~dp0\dist"

set cmake_flags=-DCMAKE_INSTALL_PREFIX=%distdir%

rem set generate_compile_commands="no" to skip generating compile_commands.json
set generate_compile_commands="yes"
if %generate_compile_commands%=="yes" (
  set cmake_flags=%cmake_flags% -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
)

if not exist "%~dp0\build" (
  mkdir "%~dp0\build" || goto :error
)

cd "%~dp0\build" && cmake -G %generator% %cmake_flags% .. || goto :cmakeerror

echo "Successfully configured CMake project"
exit /b 0

:error
  echo "Failed to create build directory"
  exit /b 1

:cmakeerror
  echo "Failed to configure CMake project"
  exit /b 2
