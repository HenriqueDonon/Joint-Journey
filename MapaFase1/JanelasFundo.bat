@echo off
rem Espera o Editor fechar, coloca o fundo luminoso atras das janelas e reabre o Editor.
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64"
set "PRJ=%~dp0..\JogoCoop.uproject"
echo Salve (Ctrl+S) e feche o Editor da Unreal. Este processo continua sozinho depois.
:espera
tasklist /FI "IMAGENAME eq UnrealEditor.exe" | find /I "UnrealEditor.exe" >nul && (timeout /t 5 >nul & goto espera)
"%UE%\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0JanelasFundo.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0janelas.log" 2>&1
start "" "%UE%\UnrealEditor.exe" "%PRJ%" /Game/Maps/Fase1_Editavel
