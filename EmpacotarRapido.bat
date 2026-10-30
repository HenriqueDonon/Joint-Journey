@echo off
setlocal
rem So empacota (sem compilar/importar). Resultado em PRONTO_AMIGO.log.
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "RES=%~dp0PRONTO_AMIGO.log"
echo INICIO %date% %time% > "%RES%"
call "%UE%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%PRJ%" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -map=MainMenu+Sala1+Fase1_Teste+Fase1+Backup/Fase1_Original_Editavel -archive -archivedirectory="%~dp0Build" -utf8output > "%~dp0empacotar.log" 2>&1
echo EMPACOTAR_EXIT %ERRORLEVEL% >> "%RES%"
echo FIM %date% %time% >> "%RES%"
