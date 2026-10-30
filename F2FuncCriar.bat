@echo off
rem Fase 2 funcional (Tarefa 05): compila e cria o mapa experimental /Game/Maps/Fase2_Funcional (nao toca no Fase2_Blockout)
call "%~dp0CompilarSilencioso.bat"
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0MapaFase2\CriarFase2Funcional.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase2\func_py.log" 2>&1
echo FIM > "%~dp0MapaFase2\func_fim.txt"
