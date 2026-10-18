@echo off
rem Convidado, sem precisar empacotar (usa o Editor). Digite o IP do Radmin do host.
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set /p IP=IP do host: 
start "" "%UE%" "%PRJ%" %IP%:7777 -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding
