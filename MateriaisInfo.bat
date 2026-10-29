@echo off
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0MapaFase1Arq\Materiais.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase1Arq\mat.log" 2>&1
