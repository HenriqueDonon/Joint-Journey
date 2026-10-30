@echo off
rem testes depois da promocao: mapa oficial /Game/Maps/Fase1 e backup do antigo
call "%~dp0CompilarSilencioso.bat"
set "A=%~dp0AutoShots"
if exist "%A%\estrutura" move "%A%\estrutura" "%A%\estrutura_refino" >nul
call "%~dp0TourFase1.bat" Fase1 estrutura
call "%~dp0FluxoFase1.bat" Fase1 -InspCam -InspPonte
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\Q_ponte" >nul
call "%~dp0FluxoFase1.bat" Fase1 -F1Salvar -InspCam
timeout /t 110 /nobreak >nul
move "%A%\f1fluxo" "%A%\Q_salvar" >nul
call "%~dp0FluxoFase1.bat" Fase1 -F1Colapso
timeout /t 95 /nobreak >nul
move "%A%\f1fluxo" "%A%\Q_colapso" >nul
call "%~dp0FluxoFase1.bat" Fase1
timeout /t 85 /nobreak >nul
copy /y "%~dp0Saved\Logs\JogoCoop.log" "%A%\f1fluxo\ue_log_a.txt" >nul
copy /y "%~dp0Saved\Logs\JogoCoop_2.log" "%A%\f1fluxo\ue_log_b.txt" >nul
move "%A%\f1fluxo" "%A%\Q_puzzle" >nul
call "%~dp0FluxoFase1.bat" Backup/Fase1_Original_Editavel
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\Q_backup" >nul
echo FIM > "%A%\Q_chain_fim.txt"
