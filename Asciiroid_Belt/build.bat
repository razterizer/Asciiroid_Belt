@echo off
call ..\..\lib\Core\build.bat

SET configuration="Release"
IF "%~1" == "Debug" SET configuration="Debug"
SET target="x64"
IF "%~2" == "x86" SET target="x86"
msbuild Asciiroid_Belt.sln /p:Configuration=%configuration% /p:Platform=%target%

if %errorlevel% neq 0 (
    echo Compilation failed with error code %errorlevel%.
    exit /b %errorlevel%
)

echo Build succeeded.
