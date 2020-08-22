@echo off
set config=%1
IF "%1" == "" set config=debug

IF EXIST .\bin\%config%\ (
  pushd .\bin\%config%\
  echo.
  start Test.exe
  echo.
  popd
)
IF EXIST .\bin\%config%\ (
  pushd .\bin\%config%\
  echo.
  start Test.exe
  echo.
  popd
)
