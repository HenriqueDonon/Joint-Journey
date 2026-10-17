@echo off
rem Compila e importa o Vertice para o mapa de teste (nao altera o mapa principal).
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0ImportVertice\setup.log"
taskkill /F /IM LiveCodingConsole.exe >nul 2>&1
echo [build] > "%LOG%"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild >> "%LOG%" 2>&1
echo BUILD_EXIT %ERRORLEVEL% >> "%LOG%"
if not "%ERRORLEVEL%"=="0" exit /b 1
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0ImportVertice\ImportarVertice.py" -unattended -nosplash -stdout -FullStdOutLogOutput >> "%LOG%" 2>&1
echo IMPORT_EXIT %ERRORLEVEL% >> "%LOG%"
