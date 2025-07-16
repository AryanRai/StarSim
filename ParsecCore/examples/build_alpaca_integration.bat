@echo off
REM Build script for Alpaca API integration
REM This script sets up dependencies and builds the live trading application

echo.
echo ========================================
echo Building Alpaca API Integration
echo ========================================
echo.

REM Check if vcpkg is available
where vcpkg >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: vcpkg not found in PATH
    echo Please install vcpkg and add it to your PATH
    echo Visit: https://github.com/Microsoft/vcpkg
    pause
    exit /b 1
)

echo Installing required packages with vcpkg...
echo.

REM Install required packages
vcpkg install curl:x64-windows
vcpkg install nlohmann-json:x64-windows
vcpkg install websocketpp:x64-windows

if %ERRORLEVEL% NEQ 0 (
    echo Error: Failed to install dependencies
    pause
    exit /b 1
)

echo.
echo Dependencies installed successfully!
echo.

REM Create build directory
if not exist "build" mkdir build
cd build

echo Configuring CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo Error: Build failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.

echo Executable location: build\Release\live_alpaca_trading.exe
echo.

echo Next steps:
echo 1. Run: python ..\examples\setup_alpaca_api.py
echo 2. Get your Alpaca API credentials
echo 3. Run: live_alpaca_trading.exe
echo.

pause