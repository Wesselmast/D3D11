@echo off
set config=%1
IF "%1" == "" set config=debug

call build-client %config%
call build-server %config%
