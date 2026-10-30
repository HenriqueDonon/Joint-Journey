@echo off
rem Fase 2 funcional: recompila, recria o mapa, capturas fixas (1 instancia) + fluxo host/cliente locais
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "A=%~dp0AutoShots"
call "%~dp0CompilarSilencioso.bat"
echo recriar > "%~dp0MapaFase2\RECRIAR_FUNC.txt"
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase2\CriarFase2Funcional.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase2\func_py.log" 2>&1
move "%~dp0MapaFase2\RECRIAR_FUNC.txt" "%~dp0MapaFase2\_RECRIAR_FUNC_usado_%RANDOM%.txt" >nul
if exist "%A%\f2vistas" move "%A%\f2vistas" "%A%\f2vistas_%RANDOM%" >nul
if exist "%A%\f2fluxo" move "%A%\f2fluxo" "%A%\f2fluxo_%RANDOM%" >nul
"%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -F2Vistas -F2SemIntro
start "F2_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F2Fluxo -FluxoTag=host
timeout /t 20 /nobreak >nul
start "F2_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F2Fluxo -FluxoTag=client
timeout /t 270 /nobreak >nul
echo FIM > "%A%\F2_fim.txt"
