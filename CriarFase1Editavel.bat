@echo off
setlocal
rem Gera o mapa editavel /Game/Maps/Fase1_Editavel (abra com EditarFase1.bat). Feche o Unreal e o jogo antes. Log: MapaFase1\editavel.log
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0MapaFase1\editavel.log"
echo INICIO %time% > "%LOG%"
taskkill /F /IM UnrealEditor.exe /IM UnrealEditor-Cmd.exe >nul 2>&1
call "%UE%\Engine\Build\BatchFiles\Build.bat" JogoCoopEditor Win64 Development -Project="%PRJ%" -WaitMutex -FromMsBuild > "%~dp0compilar.log" 2>&1
echo COMPILE_EXIT %ERRORLEVEL% >> "%LOG%"
if not "%ERRORLEVEL%"=="0" exit /b 1
del /q "%~dp0MapaFase1\layout.json" >nul 2>&1
"%UE%\Engine\Binaries\Win64\UnrealEditor.exe" "%PRJ%" /Game/Maps/Fase1_Teste?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -DumpLayout
if not exist "%~dp0MapaFase1\layout.json" (echo SEM_LAYOUT >> "%LOG%" & exit /b 1)
echo LAYOUT_OK >> "%LOG%"
"%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PRJ%" -ExecutePythonScript="%~dp0MapaFase1\CriarFase1Editavel.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase1\editavel_py.log" 2>&1
findstr /C:"[FASE1ED]" "%~dp0MapaFase1\editavel_py.log" >> "%LOG%"
echo FIM %time% >> "%LOG%"
