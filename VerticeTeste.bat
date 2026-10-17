@echo off
rem Abre o MAPA DE TESTE do Vertice (WASD andar, Shift correr, Espaco pular, mouse camera).
rem Para a demonstracao automatica com capturas: VerticeTeste.bat demo
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "EXTRA="
if /I "%1"=="demo" set "EXTRA=-VerticeDemo"
start "" "%UE%\Engine\Binaries\Win64\UnrealEditor.exe" "%PRJ%" /Game/Vertice/Maps/Teste_Vertice -game -windowed -ResX=1280 -ResY=720 -WinX=80 -WinY=60 -log -NoLiveCoding %EXTRA%
