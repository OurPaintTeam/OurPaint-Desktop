@echo off
cd /d "%~dp0"
xcopy "..\Rendering\renderer\shaders" "..\..\cmake-build-debug\src\app\shaders" /E /I /Y
xcopy "..\Rendering\renderer\fonts" "..\..\cmake-build-debug\src\app\fonts" /E /I /Y