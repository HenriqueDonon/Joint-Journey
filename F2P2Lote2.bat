@echo off
rem Fase 2 / P2 - lote 2: recompila (rearme animado), sessao n3, sessao de video, vistas do P2, teste do P1 e regressoes
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "A=%~dp0AutoShots"
call "%~dp0CompilarSilencioso.bat"
call :sessao n3 ""
call :sessao video2 "-InspP2"
if exist "%A%\f2vistas" move "%A%\f2vistas" "%A%\f2vistas_old_%RANDOM%" >nul
"%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -F2Vistas -F2VistasP2 -F2SemIntro
move "%A%\f2vistas" "%A%\P2_vistas" >nul
if exist "%A%\f2p1" move "%A%\f2p1" "%A%\f2p1_old_%RANDOM%" >nul
start "P1_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F2P1 -FluxoTag=host
timeout /t 20 /nobreak >nul
start "P1_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F2P1 -FluxoTag=client
timeout /t 225 /nobreak >nul
move "%A%\f2p1" "%A%\P2_teste_P1" >nul
if exist "%A%\elev" move "%A%\elev" "%A%\elev_%RANDOM%" >nul
call "%~dp0Fase2ElevTeste.bat"
timeout /t 200 /nobreak >nul
move "%A%\elev" "%A%\P2R_elevteste" >nul
call "%~dp0FluxoFase1.bat" Fase1
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\P2R_fase1" >nul
echo FIM > "%A%\P2B_fim.txt"
goto :eof
:sessao
if exist "%A%\f2p2" move "%A%\f2p2" "%A%\f2p2_old_%RANDOM%" >nul
start "P2_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F2P2 -FluxoTag=host %~2
timeout /t 20 /nobreak >nul
start "P2_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F2P2 -FluxoTag=client %~2
timeout /t 255 /nobreak >nul
copy /y "%~dp0Saved\Logs\JogoCoop.log" "%A%\f2p2\ue_host.txt" >nul
move "%A%\f2p2" "%A%\P2_%1" >nul
goto :eof
