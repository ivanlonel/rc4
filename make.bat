@echo off

REM Compiler path. May contain the 'make' application and other utilities as well.
set CC_PATH=C:\mingw-w64\x86_64-6.2.0-release-posix-seh-rt_v5-rev1\mingw64\bin
::set CC_PATH=C:\mingw-w64\x86_64-5.4.0-release-posix-seh-rt_v5-rev0\mingw64\bin
::set CC_PATH=C:\mingw-w64\x86_64-4.9.3-release-posix-seh-rt_v4-rev1\mingw64\bin
::set CC_PATH=C:\Dev-Cpp\MinGW64\bin
::set CC_PATH=C:\cygwin64\bin

REM Where to look for executables mentioned in the makefile, like 'rm', 'mv', 'grep', 'uname' and even 'make' itself if they aren't in CC_PATH already
set UTILS_PATH=C:\msys\bin
::set UTILS_PATH=C:\gnuwin32\bin
::set UTILS_PATH=C:\unxutils\usr\local\wbin

REM Name of the 'make' application, to be found within the PATH.
set MK_EXEC=mingw32-make.exe
::set MK_EXEC=make.exe

REM Appending paths to PATH (instead of prepending) so that Windows keeps using C:\Windows\System32\find.exe instead of GNU 'find' on next runs.
echo ;%PATH%; | find /i /c ";%CC_PATH%;" >nul || set PATH=%PATH%;%CC_PATH%
echo ;%PATH%; | find /i /c ";%UTILS_PATH%;" >nul || set PATH=%PATH%;%UTILS_PATH%

REM Call 'make' with all the arguments passed to this batch script
%MK_EXEC% BINNAME=rc4.exe %*