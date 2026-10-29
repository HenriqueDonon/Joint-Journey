@echo off
setlocal
rem Compila e cria a copia experimental /Game/Maps/Fase1_Experimental (mapa oficial intacto). Log: MapaFase1X\criar.log
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0MapaFase1X\criar.log"
echo INICIO %time% > "%LOG%"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% >> "%LOG%"
if not "%ERRORLEVEL%"=="0" exit /b 1
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase1X\CriarFase1Experimental.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase1X\criar_py.log" 2>&1
findstr /C:"[F1X]" /C:"Error:" "%~dp0MapaFase1X\criar_py.log" >> "%LOG%"
echo FIM %time% >> "%LOG%"
