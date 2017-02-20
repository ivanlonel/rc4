@echo off

REM Compiler path. May contain the 'make' application and other utilities as well.
set CC_PATH=C:\msys64\mingw64\bin
::set CC_PATH=C:\mingw-w64\x86_64-6.3.0-release-posix-seh-rt_v5-rev1\mingw64\bin
::set CC_PATH=C:\mingw-w64\x86_64-5.4.0-release-posix-seh-rt_v5-rev0\mingw64\bin
::set CC_PATH=C:\mingw-w64\x86_64-4.9.3-release-posix-seh-rt_v4-rev1\mingw64\bin
::set CC_PATH=C:\Dev-Cpp\MinGW64\bin
::set CC_PATH=C:\CodeBlocks\MinGW\bin
::set CC_PATH=C:\cygwin64\bin

REM Where to look for executables called from the makefile, like 'rm', 'mv', 'grep', 'uname' and even 'make' itself if they aren't in CC_PATH already
set UTILS_PATH=C:\msys64\usr\bin
::set UTILS_PATH=C:\msys\bin
::set UTILS_PATH=C:\gnuwin32\bin
::set UTILS_PATH=C:\unxutils\usr\local\wbin

REM Name of the 'make' application, to be found within the PATH.
set MK_EXEC=mingw32-make.exe
::set MK_EXEC=make.exe

REM Appending paths to PATH (instead of prepending) so that Windows keeps using %SystemRoot%\System32\find.exe instead of GNU 'find' on next runs.
echo ;%PATH%; | find /i /c ";%CC_PATH%;" >nul || set PATH=%PATH%;%CC_PATH%
echo ;%PATH%; | find /i /c ";%UTILS_PATH%;" >nul || set PATH=%PATH%;%UTILS_PATH%

REM Call 'make' with all the arguments passed to this batch script, plus BINNAME with an explicit .exe extension and a minimal PATH to avoid ambiguity with Windows utility names (like 'find')
%MK_EXEC% BINNAME=rc4.exe PATH="%CC_PATH%;%UTILS_PATH%" %*


REM cl /nologo /MP /Za /Wall /analyze /D_CRT_SECURE_NO_WARNINGS /DKEY_LENGTH_REMAINDER_BITWISE_AND /Iinclude src\*.c /Fo.o\ /Febin\rc4MT.exe /MT /Ox /GA /Gw /Gy /GL
REM cl /nologo /MP /Za /Wall /analyze /D_CRT_SECURE_NO_WARNINGS /DKEY_LENGTH_REMAINDER_BITWISE_AND /Iinclude src\*.c /Fo.o\ /Febin\rc4MD.exe /MD /Ox /GA /Gw /Gy /GL
REM cl /nologo /MP /Za /Wall /analyze /D_CRT_SECURE_NO_WARNINGS /DKEY_LENGTH_REMAINDER_BITWISE_AND /Iinclude src\*.c /Fo.o\ /Febin\rc4MTd.exe /MTd /Z7 /DEBUG
REM cl /nologo /MP /Za /Wall /analyze /D_CRT_SECURE_NO_WARNINGS /DKEY_LENGTH_REMAINDER_BITWISE_AND /Iinclude src\*.c /Fo.o\ /Febin\rc4MDd.exe /MDd /Z7 /DEBUG
