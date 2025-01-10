# Középhaladó Számítógépes Grafika

## CONFIGURING

Run `configure.bat` on Windows and `configure` on Linux

By default `cmake` is set up to configure and install git hooks if it recognises
it is in a git repository, if you want to disable this behaviour, run the
configure script with the `-DDISABLE_GIT_HOOKS` argument.

## BUILDING

With Microsoft Visual Studio C/C++ (generator: Visual Studio 17 2022)  
`cmake --build . --target ALL_BUILD --config RelWithDebInfo -- /maxcpucount`  
alternatively, build the `ALL_BUILD` target in Visual Studio

With GCC/Clang (generator: Ninja)  
`ninja all`

With GCC/Clang (generator: Unix Makefiles)  
`make -j$(nproc) all`

---

TEDLHY - (2024/25/01 félév)
