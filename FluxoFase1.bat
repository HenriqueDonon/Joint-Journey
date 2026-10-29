@echo off
rem Teste automatico do puzzle da Fase 1 com host + cliente LOCAIS (mesma maquina). Uso: FluxoFase1.bat <Mapa>  -> AutoShots\f1fluxo\
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
start "F1_HOST" "%EXE%" "%PRJ%" /Game/Maps/%1?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F1Fluxo -FluxoTag=host
timeout /t 25 /nobreak >nul
start "F1_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F1Fluxo -FluxoTag=client
