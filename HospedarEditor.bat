@echo off
rem Host, sem precisar empacotar (usa o Editor). Rode o Radmin VPN antes.
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
start "" "%UE%" "%PRJ%" -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding
