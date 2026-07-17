@echo off
cd /d "%~dp0"
xcopy "..\render\render\shaders" "..\..\cmake-build-debug\src\app\shaders" /E /I /Y
xcopy "..\render\render\fonts" "..\..\cmake-build-debug\src\app\fonts" /E /I /Y