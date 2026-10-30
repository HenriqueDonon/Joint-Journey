@echo off
rem Fase 2 / Puzzle 1: recompila, recria o Fase2_Funcional, e roda 3 sessoes host+cliente locais (2 normais + 1 com papeis invertidos)
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "A=%~dp0AutoShots"
call "%~dp0CompilarSilencioso.bat"
echo recriar > "%~dp0MapaFase2\RECRIAR_FUNC.txt"
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase2\CriarFase2Funcional.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase2\func_py.log" 2>&1
move "%~dp0MapaFase2\RECRIAR_FUNC.txt" "%~dp0MapaFase2\_RECRIAR_FUNC_usado_%RANDOM%.txt" >nul
for %%S in (s1 s2 swap) do call :sessao %%S
echo FIM > "%A%\P1_fim.txt"
goto :eof
:sessao
if exist "%A%\f2p1" move "%A%\f2p1" "%A%\f2p1_old_%RANDOM%" >nul
set "X="
if "%1"=="swap" set "X=-F2Swap"
start "P1_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F2P1 -FluxoTag=host %X%
timeout /t 20 /nobreak >nul
start "P1_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F2P1 -FluxoTag=client %X%
timeout /t 225 /nobreak >nul
copy /y "%~dp0Saved\Logs\JogoCoop.log" "%A%\f2p1\ue_host.txt" >nul
move "%A%\f2p1" "%A%\P1_%1" >nul
goto :eof
