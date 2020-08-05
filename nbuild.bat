set config=%1
IF "%1" == "" set config=debug

call build-client %config%
call build-server %config%

IF EXIST .\bin\client\%config%\ (
  pushd .\bin\client\%config%\
  echo.
  start Test.exe
  echo.
  popd
)
IF EXIST .\bin\server\%config%\ (
  pushd .\bin\server\%config%\
  echo.
  start Test.exe
  echo.
  popd
)
