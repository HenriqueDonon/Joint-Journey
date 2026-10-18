@echo off
rem Host. Jogo empacotado: JogoCoop.exe na mesma pasta. Rode o Radmin VPN antes.
rem Abre no menu principal - escolha "Iniciar Campanha" > 2 ou 3 jogadores para hospedar.
start "" "%~dp0JogoCoop.exe" -log -windowed -ResX=1280 -ResY=720
