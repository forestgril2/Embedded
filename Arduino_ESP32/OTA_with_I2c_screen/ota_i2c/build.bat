@echo off

:: Check if platformio is installed
where pio >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo PlatformIO is not installed. Please install it first.
    exit /b 1
)

:: Build the project
echo Building project...
pio run
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)
echo Build successful!

:: Ask for deployment method
echo Choose deployment method:
echo 1) Serial upload (USB)
echo 2) OTA upload (WiFi)
set /p choice="Enter your choice (1 or 2): "

if "%choice%"=="1" (
    echo Uploading via serial...
    pio run -t upload
) else if "%choice%"=="2" (
    echo Uploading via OTA...
    pio run -e esp32dev_ota -t upload
) else (
    echo Invalid choice!
    exit /b 1
)

:: Check if upload was successful
if %ERRORLEVEL% equ 0 (
    echo Upload successful!
    set /p monitor="Do you want to monitor serial output? (y/n): "
    if /i "%monitor%"=="y" (
        echo Starting serial monitor...
        pio device monitor
    )
) else (
    echo Upload failed!
    exit /b 1
) 