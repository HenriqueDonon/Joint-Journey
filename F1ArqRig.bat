@echo off
call "%~dp0CompilarSilencioso.bat"
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0MapaFase1Arq\CriarFase1Arquitetura.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0MapaFase1Arq\criar_py.log" 2>&1
findstr /C:"[F1A]" /C:"Error:" "%~dp0MapaFase1Arq\criar_py.log" > "%~dp0MapaFase1Arq\criar.log"
call "%~dp0TourFase1.bat" Fase1_Arquitetura estrutura
