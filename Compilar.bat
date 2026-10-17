@echo off
setlocal
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0compilar.log"
if not exist "%UE%\Engine\Build\BatchFiles\Build.bat" (
  echo Unreal 5.5 nao encontrado em %UE%
  echo Unreal 5.5 nao encontrado em %UE% > "%LOG%"
  pause & exit /b 1
)
taskkill /F /IM UnrealEditor.exe /IM UnrealEditor-Cmd.exe /IM LiveCodingConsole.exe /IM CrashReportClientEditor.exe >nul 2>&1
timeout /t 3 /nobreak >nul
echo [1/2] Compilando o jogo (pode levar 10-20 min na primeira vez)...
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%LOG%" 2>&1
set ERR=%ERRORLEVEL%
echo Codigo de saida da compilacao: %ERR% >> "%LOG%"
if not "%ERR%"=="0" (
  echo.
  echo FALHOU na compilacao. Avise o Claude - o log esta em compilar.log
  pause & exit /b %ERR%
)
echo [2/2] Criando o mapa Sala1...
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0CriarMapa.py" -unattended -nosplash -stdout -FullStdOutLogOutput >> "%LOG%" 2>&1
echo Codigo de saida do mapa: %ERRORLEVEL% >> "%LOG%"
echo.
echo PRONTO. Avise o Claude. (Log: compilar.log)
pause
