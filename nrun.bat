@echo off
set config=%1
IF "%1" == "" set config=debug

IF EXIST .\bin\server\%config%\ (
  pushd .\bin\server\%config%\
  echo.
  start Test.exe
  echo.
  popd
)
timeout 1 >nul
IF EXIST .\bin\client\%config%\ (
  pushd .\bin\client\%config%\
  echo.
  start Test.exe
  echo.
  popd
)
