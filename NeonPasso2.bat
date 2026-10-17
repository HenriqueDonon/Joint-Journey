@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0ImportNeon\passo2.log"
echo inicio > "%LOG%"
"%UE%" "%PRJ%" -ExecutePythonScript="%~dp0ImportNeon\%1" -unattended -nosplash -stdout -FullStdOutLogOutput >> "%LOG%" 2>&1
echo PASSO_FIM >> "%LOG%"
