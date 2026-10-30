@echo off
rem Desfaz a promocao da Fase 1 (nomes originais dos mapas). Depois: git revert do commit da promocao para voltar bats/config.
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0MapaFase1Arq\ReverterPromocao.py" -unattended -nosplash -stdout > "%~dp0MapaFase1Arq\reverter.log" 2>&1
