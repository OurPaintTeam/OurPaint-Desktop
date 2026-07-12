@echo off
cd /d "%~dp0"
xcopy "..\renderer\shaders" "cmake-build-debug\shaders" /E /I /Y