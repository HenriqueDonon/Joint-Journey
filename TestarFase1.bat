@echo off
rem Abre o mapa de teste da Fase 1 em 1 janela (host). F3 troca de personagem, F4 liga/desliga a ponte completa, F poder.
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
start "" "%UE%" "%PRJ%" /Game/Maps/Fase1_Teste?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding
