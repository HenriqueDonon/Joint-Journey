@echo off
setlocal
rem Compila + importa modelos do Blender + empacota (sem pausas). Resultado em PRONTO_AMIGO.log. Feche o Unreal antes.
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "RES=%~dp0PRONTO_AMIGO.log"
echo INICIO %date% %time% > "%RES%"
taskkill /F /IM UnrealEditor.exe /IM UnrealEditor-Cmd.exe /IM LiveCodingConsole.exe /IM CrashReportClientEditor.exe >nul 2>&1
timeout /t 3 /nobreak >nul
powershell -NoProfile -Command "Get-ChildItem -Path '%~dp0Source' -Recurse -File | ForEach-Object { $_.LastWriteTime = Get-Date }" >nul 2>&1
echo [1/3] compilando... >> "%RES%"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
if not "%ERRORLEVEL%"=="0" ( echo FALHOU compilacao %ERRORLEVEL% >> "%RES%" & exit /b 1 )
echo [2/3] importando modelos... >> "%RES%"
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0ImportFase1\ImportarFase1.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0ImportFase1\import.log" 2>&1
findstr /C:"[FASE1]" "%~dp0ImportFase1\import.log" >> "%RES%"
echo [3/3] empacotando... >> "%RES%"
call "%UE%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%PRJ%" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -map=MainMenu+Sala1+Fase1_Teste -archive -archivedirectory="%~dp0Build" -utf8output > "%~dp0empacotar.log" 2>&1
echo EMPACOTAR_EXIT %ERRORLEVEL% >> "%RES%"
echo FIM %date% %time% >> "%RES%"
