@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0ImportVertice\reimport.log"
echo inicio > "%LOG%"
"%UE%" "%PRJ%" -ExecutePythonScript="%~dp0ImportVertice\ImportarVertice.py" -unattended -nosplash -stdout -FullStdOutLogOutput >> "%LOG%" 2>&1
"%UE%" "%PRJ%" -ExecutePythonScript="%~dp0ImportVertice\RetargetVertice.py" -unattended -nosplash -stdout -FullStdOutLogOutput >> "%LOG%" 2>&1
"%UE%" "%PRJ%" -ExecutePythonScript="%~dp0ImportVertice\AjustesVertice.py" -unattended -nosplash -stdout -FullStdOutLogOutput >> "%LOG%" 2>&1
echo REIMPORT_FIM >> "%LOG%"
