@echo off
rem correcoes finais da Fase1_Arquitetura: testes antes da promocao
call "%~dp0CompilarSilencioso.bat"
set "A=%~dp0AutoShots"
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -InspCam -InspPonte
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\P_ponte" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -F1Salvar -InspCam
timeout /t 110 /nobreak >nul
move "%A%\f1fluxo" "%A%\P_salvar" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -F1Colapso -InspCam
timeout /t 95 /nobreak >nul
move "%A%\f1fluxo" "%A%\P_colapso" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura -F1Atalho
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\P_atalho" >nul
call "%~dp0FluxoFase1.bat" Fase1_Arquitetura
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\P_puzzle" >nul
call "%~dp0FluxoFase1.bat" Fase1_Editavel
timeout /t 85 /nobreak >nul
move "%A%\f1fluxo" "%A%\P_oficial" >nul
echo FIM > "%A%\P_chain_fim.txt"
