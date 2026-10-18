@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
del /q "%~dp0Saved\Screenshots\WindowsEditor\*" >nul 2>&1
"%UE%" "%PRJ%" /Game/Maps/Fase1_Teste?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -AutoShot
echo FIM > "%~dp0AutoShot.done"
