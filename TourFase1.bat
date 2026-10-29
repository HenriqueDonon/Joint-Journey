@echo off
rem Passeio de camera gravado quadro a quadro (30 fps fixos). Uso: TourFase1.bat <Mapa> <Pasta>  -> AutoShots\<Pasta>\f_*.jpg
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0JogoCoop.uproject" /Game/Maps/%1?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -F1Tour -TourTag=%2 -usefixedtimestep -fps=30
