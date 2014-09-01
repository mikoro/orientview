@echo off

if not exist %1\data (
  xcopy /E /I /Y /Q data %1\data
)

if not exist %1\orientview.exe.config (
  xcopy /Y /Q misc\windows\orientview.exe.config %1
)

if not exist %1\opencv_core249d.dll (
  xcopy /E /I /Y /Q debug-dll\* %1
)
