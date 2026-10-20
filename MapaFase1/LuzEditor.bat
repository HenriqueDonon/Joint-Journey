@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64"
set "PRJ=%~dp0..\JogoCoop.uproject"
taskkill /F /IM UnrealEditor.exe >nul 2>&1
timeout /t 3 >nul
"%UE%\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0LuzEditor.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0luz.log" 2>&1
start "" "%UE%\UnrealEditor.exe" "%PRJ%" /Game/Maps/Fase1_Editavel
