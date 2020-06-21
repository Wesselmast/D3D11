@echo off

set config=%1

IF "%1" == "" set config=all

echo.
echo Cleaning %config%...

IF "%config%" == "all" ( 
   IF EXIST .\bin\ rmdir /Q /S .\bin\
   GOTO :Done
)
IF EXIST .\bin\%config%\ (
   rmdir /Q /S .\bin\%config%\
   GOTO :Done
)
echo. && echo ERROR: %config% is not a valid or built build configuration && echo.
GOTO :eof

:Done
echo.
echo ">>>>>>>>>> SUCCESS! <<<<<<<<<<"
echo.
