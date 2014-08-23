@echo off

if not exist bin\Debug\opencv_core249d.dll (
  xcopy /E /I /Y /Q debug-dll\* bin\Debug
)
