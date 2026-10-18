@echo off
rem Importa na Unreal todos os modelos exportados do Blender em ImportFase1\<Nome>\. Feche o Editor antes.
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0ImportFase1\import.log"
"%UE%" "%PRJ%" -ExecutePythonScript="%~dp0ImportFase1\ImportarFase1.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%LOG%" 2>&1
findstr /C:"[FASE1]" "%LOG%"
pause
