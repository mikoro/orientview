@echo off

if not exist bin\Release\data (
  xcopy /E /I /Y /Q data bin\Release\data
)

if not exist bin\Release\orientview.exe.config (
  xcopy /Y /Q misc\windows\orientview.exe.config bin\Release
)
