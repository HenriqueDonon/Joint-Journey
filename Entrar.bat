@echo off
rem Convidado (Ruina). Digite o IP do Radmin do host (ex: 26.12.34.56).
set /p IP=IP do host: 
start "" "%~dp0JogoCoop.exe" %IP%:7777 -log -windowed -ResX=1280 -ResY=720
