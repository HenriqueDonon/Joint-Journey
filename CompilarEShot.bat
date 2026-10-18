@echo off
setlocal
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
echo INICIO %time% > "%~dp0CompilarEShot.log"
taskkill /F /IM UnrealEditor.exe /IM UnrealEditor-Cmd.exe >nul 2>&1
powershell -NoProfile -Command "Get-ChildItem -Path '%~dp0Source' -Recurse -File | ForEach-Object { $_.LastWriteTime = Get-Date }" >nul 2>&1
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% >> "%~dp0CompilarEShot.log"
if not "%ERRORLEVEL%"=="0" exit /b 1
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0ImportFase1\ImportarFase1.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0ImportFase1\import.log" 2>&1
echo IMPORT_EXIT %ERRORLEVEL% >> "%~dp0CompilarEShot.log"
del /q "%~dp0AutoShots\*" >nul 2>&1
"%UE%\Engine\Binaries\Win64\UnrealEditor.exe" "%PRJ%" /Game/Maps/Fase1_Teste?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -AutoShot
echo SHOT_DONE %time% >> "%~dp0CompilarEShot.log"
