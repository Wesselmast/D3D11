@echo off

set config=%1

set genericflags=-D_CRT_SECURE_NO_WARNINGS -Wall -Wno-unused-variable -c -D_GLFW_WIN32
set dflags= %genericflags% -g -DDEBUG
set rflags=%genericflags% -O2 -DRELEASE
set dLflags=-g
set rLflags=-Wl,-subsystem:windows
set linkerpaths=-LLib\\10.0.17763.0\\um\\x86\\
set includepaths=-IInclude\\10.0.17763.0\\ 
set libraries=-lopengl32 -luser32 -lgdi32 -ld3d11
set compiler=clang++
set caching=

IF NOT [%caching%] == [] (
   WHERE %caching%
   IF %ERRORLEVEL% NEQ 0 (
      echo %caching% not found, removing..
      set caching=
   )
)

IF [%1] == [] set config=debug
IF NOT [%~2] == [] for /f "tokens=1,* delims= " %%a in ("%*") do set compiler=%%b

IF NOT EXIST .\bin\%config%\ mkdir .\bin\%config%\
robocopy .\res .\bin\%config%\res /E /NFL /NDL /NJH /NJS

echo.
echo Compiler: %compiler%

echo.
echo Compiling %config%...

IF [%config%] == [debug] GOTO Debug
IF [%config%] == [release] GOTO Release
echo. && echo ERROR: %config% is not a valid build configuration && echo.
rmdir .\bin\%config%\
GOTO :eof

:Release
call duration -c %caching% %compiler% src\Windows.cpp %includepaths% %rflags% -o bin\%config%\Windows.o 
echo. && echo Linking %config%...
call duration -l %compiler% -o bin\%config%\Test.exe bin\%config%\Windows.o %linkerpaths% %libraries% %rLflags%
GOTO :Success

:Debug
call duration -c %caching% %compiler% src\Windows.cpp %includepaths% %dflags% -o bin\%config%\Windows.o 
echo. && echo Linking %config%...
call duration -l %compiler% -o bin\%config%\Test.exe bin\%config%\Windows.o %linkerpaths% %libraries% %dLflags%
GOTO :Success

:Success
del bin\%config%\Windows.o

echo.
echo ">>>>>>>>>> SUCCESS! <<<<<<<<<<"
echo.
echo.
