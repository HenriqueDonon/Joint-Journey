@echo off
rem HOST direto na Fase1_Teste (sala da ponte), usando o jogo empacotado. Rode o Radmin VPN antes. O amigo usa Entrar.bat com o IP do host.
set "EXE=%~dp0Build\Windows\JogoCoop.exe"
if not exist "%EXE%" (
  echo Nao encontrei "%EXE%". Rode ProntoParaAmigo.bat primeiro.
  pause
  exit /b 1
)
start "" "%EXE%" /Game/Maps/Fase1_Teste?listen -log -windowed -ResX=1280 -ResY=720
