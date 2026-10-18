@echo off
rem Convidado. Precisa ter o jogo empacotado (pasta Build\Windows ou recebida do host).
set "EXE=%~dp0Build\Windows\JogoCoop.exe"
if not exist "%EXE%" (
  echo Nao encontrei "%EXE%". Confirme que a pasta Build\Windows\JogoCoop.exe existe aqui.
  pause
  exit /b 1
)
set /p IP=IP do host: 
start "" "%EXE%" %IP%:7777 -log -windowed -ResX=1280 -ResY=720
