@echo off

set config=%1
IF "%1" == "" set config=debug

IF EXIST .\bin\%config%\ (
  pushd .\bin\%config%\
  echo.
  call Test.exe
  echo.
  popd
  GOTO :eof
)
echo. && echo ERROR: the %config% configuration has not been built or doesn't exist && echo.
