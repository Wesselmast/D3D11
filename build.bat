@echo off

set config=%1

set genericflags=-D_CRT_SECURE_NO_WARNINGS -c -D_GLFW_WIN32
set debugflags= %genericflags% -g -DDEBUG
set releaseflags=%genericflags% -O2 -DRELEASE
set linkerpaths=-LLib\\10.0.17763.0\\um\\x86\\
set includepaths=-IInclude\\10.0.17763.0\\
set libraries=-lopengl32 -luser32 -lgdi32 -lshell32 -lkernel32 -ld3d11
set compiler=clang++
set speedups=ccache

WHERE ccache
IF %ERRORLEVEL% NEQ 0 (
   echo ccache not found, removing..
   set speedups=
)

IF "%~1" == "" set config=debug
IF NOT "%~2" == "" set compiler = %%2

echo.
echo Building %config%...
echo.

IF NOT EXIST .\bin\%config%\ mkdir .\bin\%config%\
IF "%config%" == "debug" GOTO Debug
IF "%config%" == "release" GOTO Release
echo. && echo ERROR: %config% is not a valid build configuration && echo.
rmdir .\bin\%config%\
GOTO :eof

:Release
call duration -c %speedups% %compiler% src\Windows.cpp %includepaths% %releaseflags% -o bin\%config%\Windows.o 
GOTO Linking

:Debug
call duration -c %speedups% %compiler% src\Windows.cpp %includepaths% %debugflags% -o bin\%config%\Windows.o 
GOTO Linking

:Linking
call duration -l %speedups% %compiler% -o bin\%config%\Test.exe bin\%config%\Windows.o %linkerpaths% %libraries%

echo.
echo ">>>>>>>>>> SUCCESS! <<<<<<<<<<"
echo.
