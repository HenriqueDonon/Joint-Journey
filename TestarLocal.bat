@echo off
rem Abre 3 janelas: 1a hospeda (Neon), 2a (Ruina), 3a (Vertice). F3 em qualquer janela alterna para o Vertice (teste).
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
start "" "%UE%" "%PRJ%" /Game/Maps/Sala1?listen -game -windowed -ResX=640 -ResY=400 -WinX=0 -WinY=60 -log -NoLiveCoding
timeout /t 25 /nobreak >nul
start "" "%UE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=640 -ResY=400 -WinX=645 -WinY=60 -log -NoLiveCoding
timeout /t 10 /nobreak >nul
start "" "%UE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=640 -ResY=400 -WinX=1290 -WinY=60 -log -NoLiveCoding
