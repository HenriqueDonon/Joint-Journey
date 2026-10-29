@echo off
rem Teste multiplayer LOCAL do elevador: host (listen) + 1 cliente, ambos na mesma maquina (loopback). Args opcionais: %1 = latencia ms do cliente.
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "LAG="
if not "%1"=="" if not "%1"=="0" set "LAG=-ElevLag=%1"
mkdir "%~dp0AutoShots\elev" >nul 2>&1
start "ELEV_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_ElevTeste?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -ElevAuto -ElevTag=host %2
timeout /t 25 /nobreak >nul
start "ELEV_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -ElevAuto -ElevTag=client %LAG% %2
