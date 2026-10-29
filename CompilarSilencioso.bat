@echo off
call "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%~dp0JogoCoop.uproject" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% > "%~dp0compilar_status.txt"
