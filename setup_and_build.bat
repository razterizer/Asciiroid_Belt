@echo off

set REPO_DIR=Asciiroids

REM Change directory
cd ..

REM Run the dependency fetch script
python "%REPO_DIR%\fetch-dependencies.py" "%REPO_DIR%\dependencies"

REM Navigate to the appropriate directory
cd "%REPO_DIR%\%REPO_DIR%"

REM Run the build script
call build.bat

:askUser
REM Ask the user if they want to run the program
set /p response=Do you want to run the program? (yes/no): 

REM Process the response
if /i "%response%"=="yes" (
    echo Running the program...
    call run.bat
    goto end
) else if /i "%response%"=="no" (
    echo Alright. Have a nice day!
    goto end
) else (
    echo Invalid response. Please answer yes or no.
    goto askUser
)

:end
