@echo off
rem Abre o Editor da Unreal direto no mapa editavel da Fase 1. Salve com Ctrl+S. Teste com TestarFase1.bat.
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
if not exist "%~dp0Content\Maps\Fase1_Editavel.umap" (echo Rode CriarFase1Editavel.bat primeiro. & pause & exit /b 1)
start "" "%UE%" "%PRJ%" /Game/Maps/Fase1_Editavel
