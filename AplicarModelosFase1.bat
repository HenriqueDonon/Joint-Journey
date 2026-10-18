@echo off
rem 1) compila o codigo  2) importa os modelos do Blender  3) abre a Fase1_Teste. Feche o Unreal Editor antes.
call "%~dp0Compilar.bat"
call "%~dp0ImportarFase1.bat"
call "%~dp0TestarFase1.bat"
