@echo off
rem cadeia de testes: salvamento, colapso (camera de inspecao no host), puzzle no novo mapa, regressao no mapa oficial
call "%~dp0CompilarSilencioso.bat"
set "A=%~dp0AutoShots"
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -F1Salvar -InspCam
timeout /t 110 /nobreak >nul
move "%A%\f1fluxo" "%A%\T_salvar" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -F1Colapso -InspCam
timeout /t 95 /nobreak >nul
move "%A%\f1fluxo" "%A%\T_colapso" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura
timeout /t 80 /nobreak >nul
move "%A%\f1fluxo" "%A%\T_puzzle" >nul
call "%~dp0FluxoFase1.bat" Fase1_Editavel
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\T_oficial" >nul
echo FIM > "%A%\T_chain_fim.txt"
