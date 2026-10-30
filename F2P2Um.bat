@echo off
rem Fase 2 / Puzzle 2: compila, recria o Fase2_Funcional e roda sessoes host+cliente locais:
rem   n1 = normal | swaplag = papeis invertidos + 100 ms no cliente | n2 = nova sessao (reinicio) | video = cameras fixas nas salas | p1 = teste do P1
set "EXE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
set "A=%~dp0AutoShots"
call "%~dp0CompilarSilencioso.bat"
echo recriar > "%~dp0MapaFase2\RECRIAR_FUNC.txt"
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase2\CriarFase2Funcional.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase2\func_py.log" 2>&1
move "%~dp0MapaFase2\RECRIAR_FUNC.txt" "%~dp0MapaFase2\_RECRIAR_FUNC_usado_%RANDOM%.txt" >nul
call :sessao n1 ""
call :sessao swaplag "-F2Swap -F2Lag=100"
call :sessao n2 ""
call :sessao video "-InspP2"
echo FIM > "%A%\P2U_fim.txt"
goto :eof
:sessao
if exist "%A%\f2p2" move "%A%\f2p2" "%A%\f2p2_old_%RANDOM%" >nul
start "P2_HOST" "%EXE%" "%PRJ%" /Game/Maps/Fase2_Funcional?listen -game -windowed -ResX=960 -ResY=540 -WinX=0 -WinY=40 -log -NoLiveCoding -F2P2 -FluxoTag=host %~2
timeout /t 20 /nobreak >nul
start "P2_CLIENT" "%EXE%" "%PRJ%" 127.0.0.1 -game -windowed -ResX=960 -ResY=540 -WinX=970 -WinY=40 -log -NoLiveCoding -F2P2 -FluxoTag=client %~2
timeout /t 255 /nobreak >nul
copy /y "%~dp0Saved\Logs\JogoCoop.log" "%A%\f2p2\ue_host.txt" >nul
move "%A%\f2p2" "%A%\P2_%1" >nul
goto :eof
