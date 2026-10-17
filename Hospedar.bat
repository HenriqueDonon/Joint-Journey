@echo off
rem Host (Neon). Jogo empacotado: JogoCoop.exe na mesma pasta. Rode o Radmin VPN antes.
start "" "%~dp0JogoCoop.exe" /Game/Maps/Sala1?listen -log -windowed -ResX=1280 -ResY=720
