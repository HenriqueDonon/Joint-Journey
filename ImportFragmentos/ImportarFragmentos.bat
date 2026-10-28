@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64"
set "PRJ=%~dp0..\JogoCoop.uproject"
"%UE%\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0ImportarFragmentos.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0import.log" 2>&1
echo FIM >> "%~dp0import.log"
