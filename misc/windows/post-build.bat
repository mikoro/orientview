@echo off

if not exist bin\%1\data (
  xcopy /E /I /Y /Q data bin\%1\data
)

if not exist bin\%1\orientview.exe.config (
  xcopy /Y /Q misc\windows\orientview.exe.config bin\%1
)
