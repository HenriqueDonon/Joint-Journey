@echo off
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "LOG=%~dp0ImportVertice\build.log"
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%~dp0JogoCoop.uproject" -WaitMutex -FromMsBuild > "%LOG%" 2>&1
echo BUILD_EXIT %ERRORLEVEL% >> "%LOG%"
