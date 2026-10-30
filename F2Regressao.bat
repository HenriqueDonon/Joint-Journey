@echo off
rem regressao apos a Tarefa 05: elevador no mapa de teste antigo (modo automatico), blockout original, Fase 1 oficial
set "A=%~dp0AutoShots"
if exist "%A%\elev" move "%A%\elev" "%A%\elev_%RANDOM%" >nul
call "%~dp0Fase2ElevTeste.bat"
timeout /t 200 /nobreak >nul
move "%A%\elev" "%A%\R5_elevteste" >nul
if exist "%A%\fase2" move "%A%\fase2" "%A%\fase2_%RANDOM%" >nul
call "%~dp0Fase2Shot.bat"
move "%A%\fase2" "%A%\R5_blockout" >nul
call "%~dp0FluxoFase1.bat" Fase1
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\R5_fase1" >nul
echo FIM > "%A%\R5_fim.txt"
