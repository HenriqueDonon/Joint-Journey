@echo off
rem Fase 2 funcional: recompila e roda so o fluxo host/cliente locais
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "A=%~dp0AutoShots"
call "%~dp0CompilarSilencioso.bat"
if exist "%A%\f2fluxo" move "%A%\f2fluxo" "%A%\f2fluxo_%RANDOM%" >nul
start "F2_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F2Fluxo -FluxoTag=host
timeout /t 20 /nobreak >nul
start "F2_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F2Fluxo -FluxoTag=client
timeout /t 270 /nobreak >nul
echo FIM > "%A%\F2_fim.txt"
