@echo off

set config=%1

set genericflags=-D_CRT_SECURE_NO_WARNINGS -Wall -Wno-unused-variable -c -D_GLFW_WIN32 -DCLIENT
set dflags= %genericflags% -g -DDEBUG
set rflags=%genericflags% -O2 -DRELEASE
set dLflags=-g
set rLflags=-Wl,-subsystem:windows
set linkerpaths=-LLib\\10.0.17763.0\\um\\x86\\
set includepaths=-IInclude\\10.0.17763.0\\ 
set libraries=-lopengl32 -luser32 -lgdi32 -ld3d11 -lws2_32
set compiler=clang++
set caching=ccache

IF NOT [%caching%] == [] (
   WHERE %caching%
   IF %ERRORLEVEL% NEQ 0 (
      echo %caching% not found, removing..
      set caching=
   )
)

IF [%1] == [] set config=debug
IF NOT [%~2] == [] for /f "tokens=1,* delims= " %%a in ("%*") do set compiler=%%b

echo.
echo Building client...
echo Compiler: %compiler%

echo.
echo Compiling %config%...

IF [%config%] == [debug] GOTO Debug
IF [%config%] == [release] GOTO Release
echo. && echo ERROR: %config% is not a valid build configuration && echo.
rmdir .\bin\client\%config%\
GOTO :eof

:Release
IF NOT EXIST .\bin\client\%config%\ mkdir .\bin\client\%config%\
robocopy .\res .\bin\client\%config%\res /E /NFL /NDL /NJH /NJS
call duration -c %caching% %compiler% src\Windows.cpp %includepaths% %rflags% -o bin\client\%config%\Windows.o 
echo. && echo Linking %config%...
call duration -l %compiler% -o bin\client\%config%\Test.exe bin\client\%config%\Windows.o %linkerpaths% %libraries% %rLflags%
GOTO :Success

:Debug
IF NOT EXIST .\bin\client\%config%\ mkdir .\bin\client\%config%\
robocopy .\res .\bin\client\%config%\res /E /NFL /NDL /NJH /NJS
call duration -c %caching% %compiler% src\Windows.cpp %includepaths% %dflags% -o bin\client\%config%\Windows.o 
echo. && echo Linking %config%...
call duration -l %compiler% -o bin\client\%config%\Test.exe bin\client\%config%\Windows.o %linkerpaths% %libraries% %dLflags%
GOTO :Success

:Success
del bin\client\%config%\Windows.o

echo.
echo ">>>>>>>>>> SUCCESS! <<<<<<<<<<"
echo.
echo.
