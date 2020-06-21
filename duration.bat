@echo off

IF "%1"=="-c" GOTO Compile
IF "%1"=="-l" GOTO Link
echo ERROR: "%1" is not a valid step argument. Skipping duration calculation for this step! 
GOTO :eof

:Compile
set step=Compile Time:
GOTO Logic

:Link
set step=Linking Time:
GOTO Logic

:Logic
for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
   set /A "start=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

for /f "tokens=1,* delims= " %%a in ("%*") do set command=%%b
cmd /c %command%

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
   set /A "end=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

set /A elapsed=end-start

set /A hh=elapsed/(60*60*100), rest=elapsed%%(60*60*100), mm=rest/(60*100), rest%%=60*100, ss=rest/100, cc=rest%%100
if %hh% lss 10 set hh=0%hh%
if %mm% lss 10 set mm=0%mm%
if %ss% lss 10 set ss=0%ss%
if %cc% lss 10 set cc=0%cc%
echo %step% %hh%:%mm%:%ss%:%cc%
