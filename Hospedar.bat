@echo off
rem Host. Precisa ter rodado Empacotar.bat antes. Rode o Radmin VPN antes.
rem Abre no menu principal - escolha "Iniciar Campanha" > 2 ou 3 jogadores para hospedar.
set "EXE=%~dp0Build\Windows\JogoCoop.exe"
if not exist "%EXE%" (
  echo Nao encontrei "%EXE%". Rode Empacotar.bat primeiro.
  pause
  exit /b 1
)
start "" "%EXE%" -log -windowed -ResX=1280 -ResY=720
