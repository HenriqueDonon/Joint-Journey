@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0ImportVertice\retarget.log"
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0ImportVertice\RetargetVertice.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%LOG%" 2>&1
echo RET_EXIT %ERRORLEVEL% >> "%LOG%"
