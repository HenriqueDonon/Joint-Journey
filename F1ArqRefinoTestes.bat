@echo off
rem refino Fase1_Arquitetura: compila, passeios (estrutura/tunel), colapso, salvamento, ponte, puzzle, regressao no mapa oficial
call "%~dp0CompilarSilencioso.bat"
set "A=%~dp0AutoShots"
if exist "%A%\estrutura" move "%A%\estrutura" "%A%\estrutura_antes" >nul
call "%~dp0TourFase1.bat" Fase1_Arquitetura estrutura
call "%~dp0TourFase1.bat" Fase1_Arquitetura tunel
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -F1Colapso -InspCam
timeout /t 95 /nobreak >nul
move "%A%\f1fluxo" "%A%\R_colapso" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -F1Salvar -InspCam
timeout /t 110 /nobreak >nul
move "%A%\f1fluxo" "%A%\R_salvar" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -InspCam -InspPonte
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\R_ponte" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\R_puzzle" >nul
call "%~dp0FluxoFase1.bat" Fase1_Editavel
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\R_oficial" >nul
echo FIM > "%A%\R_chain_fim.txt"
