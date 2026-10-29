@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
echo INICIO %time% > "%~dp0ColunaVideo.log"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% >> "%~dp0ColunaVideo.log"
if not "%ERRORLEVEL%"=="0" exit /b 1
"%UE%\Engine\Binaries\Win64\UnrealEditor.exe" "%PRJ%" /Game/Maps/Fase1_Editavel?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -ColunaVideo -usefixedtimestep -fps=30
echo SHOT_DONE %time% >> "%~dp0ColunaVideo.log"
