@echo off
call "%~dp0CompilarSilencioso.bat"
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0MapaFase1Arq\Refino.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase1Arq\refino_py.log" 2>&1
echo FIM > "%~dp0MapaFase1Arq\refino_fim.txt"
