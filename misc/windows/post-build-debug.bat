@echo off

if not exist bin\Debug\data (
  xcopy /E /I /Y /Q data bin\Debug\data
)

if not exist bin\Debug\orientview.exe.config (
  xcopy /Y /Q misc\windows\orientview.exe.config bin\Debug
)

if not exist bin\Debug\opencv_core249d.dll (
  xcopy /E /I /Y /Q debug-dll\* bin\Debug
)
