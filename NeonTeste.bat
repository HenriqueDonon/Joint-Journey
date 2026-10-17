@echo off
rem Teste do Neon no mapa de teste. Use "NeonTeste.bat demo" para a demo automatica com capturas.
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64"
set "PRJ=%~dp0JogoCoop.uproject"
set "EXTRA="
if /I "%1"=="demo" set "EXTRA=-VerticeDemo"
start "" "%UE%\UnrealEditor.exe" "%PRJ%" /Game/Vertice/Maps/Teste_Vertice -game -windowed -ResX=1280 -ResY=720 -WinX=80 -WinY=60 -log -NoLiveCoding -NeonTeste %EXTRA%
