@echo off

if not exist bin\Debug\opencv_core249d.dll (
  xcopy /E /I /Y /Q misc\debug-dll\* bin\Debug
)
