@echo off
rem Abre so o menu principal pra testar (depois de rodar MenuSetup.bat uma vez).
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
start "" "%UE%" "%PRJ%" /Game/Maps/MainMenu -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding
