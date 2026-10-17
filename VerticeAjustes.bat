@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5"
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0ImportVertice\AjustesVertice.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0ImportVertice\ajustes.log" 2>&1
echo AJ_EXIT %ERRORLEVEL% >> "%~dp0ImportVertice\ajustes.log"
