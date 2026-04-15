@echo off
setlocal

set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
if not exist %VCVARS% set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
if not exist %VCVARS% (
    echo [ERROR] Visual Studio not found!
    pause & exit /b 1
)

echo [*] Initializing MSVC x64 environment...
call %VCVARS% x64 >nul 2>&1

echo [*] Compiling...
cd /d "%~dp0src"

cl main.cpp overlay.cpp memory.cpp render.cpp ^/link d3d11.lib dxgi.lib d3dcompiler.lib user32.lib gdi32.lib Psapi.lib /out:..\overlay.exe

if %ERRORLEVEL% == 0 (
    echo [OK] Build successful! overlay.exe is in mc-overlay\
) else (
    echo [FAIL] Build failed.
)
pause
