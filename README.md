# Középhaladó Számítógépes Grafika

## CONFIGURING

> run `configure.bat` on Windows and `configure` on Linux

## BUILDING

> With Microsoft Visual Studio C/C++ (generator: Visual Studio 17 2022)
> `cmake --build . --target ALL_BUILD --config <Release|Debug|RelWithDebInfo|MinSizeRel> -- /maxcpucount`
> alternatively, build the `ALL_BUILD` target in Visual Studio
>
> With GCC/Clang (generator: Ninja)
> `ninja all`
>
> With GCC/Clang (generator: Unix Makefiles)
> `make -j$(nproc) all`

---

TEDLHY - (2024/25/01 félév)
