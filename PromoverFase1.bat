@echo off
rem Promove Fase1_Arquitetura a Fase 1 oficial (/Game/Maps/Fase1) e guarda o mapa antigo em /Game/Maps/Backup. Feche o Unreal antes.
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0MapaFase1Arq\Promover.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase1Arq\promover.log" 2>&1
echo FIM > "%~dp0MapaFase1Arq\promover_fim.txt"
