@echo off

set config=%1

set genericflags=-D_CRT_SECURE_NO_WARNINGS -Wall -c -D_GLFW_WIN32
set debugflags= %genericflags% -gcodeview -DDEBUG
set releaseflags=%genericflags% -O2 -DRELEASE
set linkerpaths=-LLib\\10.0.17763.0\\um\\x86\\
set includepaths=-IInclude\\10.0.17763.0\\
set libraries=-lopengl32 -luser32 -lgdi32 -ld3d11
set compiler=clang++
set caching=ccache

WHERE %caching%
IF %ERRORLEVEL% NEQ 0 (
   echo %caching% not found, removing..
   set caching=
)

IF "%1" == "" set config=debug
IF NOT "%~2"== "" for /f "tokens=1,* delims= " %%a in ("%*") do set compiler=%%b

echo %compiler%

echo.
echo Building %config%...

IF NOT EXIST .\bin\%config%\ mkdir .\bin\%config%\
IF EXIST .\bin\%config%\res echo.
IF NOT EXIST .\bin\%config%\res (
   robocopy .\res .\bin\%config%\res /E /NFL /NDL /NJH /NJS
)
IF "%config%" == "debug" GOTO Debug
IF "%config%" == "release" GOTO Release
echo. && echo ERROR: %config% is not a valid build configuration && echo.
rmdir .\bin\%config%\
GOTO :eof

:Release
call duration -c %caching% %compiler% src\Windows.cpp %includepaths% %releaseflags% -o bin\%config%\Windows.o 
GOTO Linking

:Debug
call duration -c %caching% %compiler% src\Windows.cpp %includepaths% %debugflags% -o bin\%config%\Windows.o 
GOTO Linking

:Linking
call duration -l %compiler% -o bin\%config%\Test.exe bin\%config%\Windows.o %linkerpaths% %libraries%

echo.
echo ">>>>>>>>>> SUCCESS! <<<<<<<<<<"
echo.
