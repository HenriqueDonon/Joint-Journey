@echo off
rem Fase 2 / Puzzle 1: sessao gravada para o video (host com camera de inspecao) + regressao do elevador antigo e da Fase 1
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "A=%~dp0AutoShots"
call "%~dp0CompilarSilencioso.bat"
if exist "%A%\f2p1" move "%A%\f2p1" "%A%\f2p1_old_%RANDOM%" >nul
start "P1_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F2P1 -FluxoTag=host -InspP1
timeout /t 20 /nobreak >nul
start "P1_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F2P1 -FluxoTag=client
timeout /t 225 /nobreak >nul
move "%A%\f2p1" "%A%\P1_video" >nul
if exist "%A%\elev" move "%A%\elev" "%A%\elev_%RANDOM%" >nul
call "%~dp0Fase2ElevTeste.bat"
timeout /t 200 /nobreak >nul
move "%A%\elev" "%A%\P1R_elevteste" >nul
call "%~dp0FluxoFase1.bat" Fase1
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\P1R_fase1" >nul
echo FIM > "%A%\P1V_fim.txt"
