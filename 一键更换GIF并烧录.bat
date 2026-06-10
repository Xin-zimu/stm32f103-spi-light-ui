@echo off
chcp 65001 >nul
cd /d "%~dp0"

where py >nul 2>nul
if %errorlevel% equ 0 (
    py -B "Tools\simple_gif_flash.py"
) else (
    python -B "Tools\simple_gif_flash.py"
)

if errorlevel 1 (
    echo.
    echo 操作未完成，请查看弹窗中的错误说明。
    pause
)
