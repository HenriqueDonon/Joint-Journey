@echo off
rem Compila e cria o mapa /Game/Maps/Fase1_Teste. Log: MapaFase1\setup.log
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0MapaFase1\setup.log"
taskkill /F /IM UnrealEditor.exe /IM UnrealEditor-Cmd.exe /IM LiveCodingConsole.exe >nul 2>&1
timeout /t 3 /nobreak >nul
powershell -NoProfile -Command "Get-ChildItem -Path '%~dp0Source' -Recurse -File | ForEach-Object { $_.LastWriteTime = Get-Date }" >nul 2>&1
echo [build] > "%LOG%"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild >> "%LOG%" 2>&1
echo BUILD_EXIT %ERRORLEVEL% >> "%LOG%"
if not "%ERRORLEVEL%"=="0" ( echo Build falhou - veja MapaFase1\setup.log & pause & exit /b 1 )
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase1\CriarMapaFase1.py" -unattended -nosplash -stdout -FullStdOutLogOutput >> "%LOG%" 2>&1
echo MAPA_EXIT %ERRORLEVEL% >> "%LOG%"
echo Pronto. Agora rode TestarFase1.bat
pause
