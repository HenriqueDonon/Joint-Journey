@echo off
setlocal
set "UE=C:\Program Files\Epic Games\UE_5.5"
set "PRJ=%~dp0JogoCoop.uproject"
set "LOG=%~dp0empacotar.log"
echo Empacotando o jogo para Windows (pode levar 20-40 min)...
call "%UE%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%PRJ%" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -map=MainMenu+Sala1+Fase1_Teste -archive -archivedirectory="%~dp0Build" -utf8output > "%LOG%" 2>&1
echo Codigo de saida: %ERRORLEVEL% >> "%LOG%"
echo Concluido. Pasta: %~dp0Build  - Avise o Claude.
pause
