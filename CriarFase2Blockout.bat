@echo off
setlocal
rem Compila e gera o mapa /Game/Maps/Fase2_Blockout (nao toca na Fase 1). Log: MapaFase2\criar.log
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0MapaFase2\criar.log"
echo INICIO %time% > "%LOG%"
taskkill /F /IM UnrealEditor.exe /IM UnrealEditor-Cmd.exe >nul 2>&1
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% >> "%LOG%"
if not "%ERRORLEVEL%"=="0" exit /b 1
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase2\CriarFase2Blockout.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase2\criar_py.log" 2>&1
findstr /C:"[FASE2]" /C:"Error:" "%~dp0MapaFase2\criar_py.log" >> "%LOG%"
echo FIM %time% >> "%LOG%"
