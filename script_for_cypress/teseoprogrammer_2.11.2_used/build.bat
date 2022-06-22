@echo off
set MINGW_HOME=C:\STGNSSTools\compilers\mingw32
build\tools\make.exe distclean
build\tools\make.exe -j all
pause