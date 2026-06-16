@echo off
SET configuration="Release"
IF "%~1" == "Debug" SET configuration="Debug"
SET target="x64"
IF "%~2" == "x86" SET target="Win32"

call ..\..\lib\Core\build.bat %~2
if %errorlevel% neq 0 exit /b %errorlevel%

msbuild Asciiroid_Belt.sln /p:Configuration=%configuration% /p:Platform=%target%

if %errorlevel% neq 0 (
    echo Compilation failed with error code %errorlevel%.
    exit /b %errorlevel%
)

echo Build succeeded.
