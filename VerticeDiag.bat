@echo off
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0JogoCoop.uproject" -ExecutePythonScript="%~dp0ImportVertice\Diag.py" -unattended -nosplash -stdout -FullStdOutLogOutput > "%~dp0ImportVertice\diag.log" 2>&1
echo DIAG_EXIT >> "%~dp0ImportVertice\diag.log"
