@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
echo INICIO %time% > "%~dp0EstadoAtual.log"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% >> "%~dp0EstadoAtual.log"
"%UE%\Engine\Binaries\Win64\UnrealEditor.exe" "%PRJ%" /Game/Maps/Backup/Fase1_Original_Editavel?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -AutoShot
echo SHOT_DONE %time% >> "%~dp0EstadoAtual.log"
