@echo off
call "%~dp0CompilarSilencioso.bat"
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0JogoCoop.uproject" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -F2Vistas -F2Intro -usefixedtimestep -fps=30
echo FIM > "%~dp0AutoShots\F2I_fim.txt"
