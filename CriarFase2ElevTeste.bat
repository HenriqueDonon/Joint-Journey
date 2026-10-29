@echo off
setlocal
rem Compila e cria o mapa isolado /Game/Maps/Fase2_ElevTeste. Log: MapaFase2\elev_criar.log
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0MapaFase2\elev_criar.log"
echo INICIO %time% > "%LOG%"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% >> "%LOG%"
if not "%ERRORLEVEL%"=="0" exit /b 1
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase2\CriarFase2ElevTeste.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase2\elev_py.log" 2>&1
findstr /C:"[ELEV]" /C:"Error:" "%~dp0MapaFase2\elev_py.log" >> "%LOG%"
echo FIM %time% >> "%LOG%"
