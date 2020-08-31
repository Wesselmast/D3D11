@echo off

set config=%1
IF "%1" == "" set config=debug

IF EXIST .\bin\%config%\res\levels\ (
   robocopy .\bin\%config%\res\levels\ .\res\levels\ /E /NFL /NDL /NJH /NJS
   goto :eof
)
echo. && echo ERROR: the %config% configuration has not been built or doesn't exist && echo.
