@echo off

set CC_PATH=C:\mingw-w64\x86_64-6.2.0-release-posix-seh-rt_v5-rev1\mingw64\bin
::set CC_PATH=C:\Dev-Cpp\MinGW64\bin
::set CC_PATH=C:\cygwin64\bin

set MK_EXEC=mingw32-make.exe
::set MK_EXEC=make.exe

REM Appending CC_PATH to PATH (instead of prepending) so that Windows keeps using C:\Windows\System32\find.exe instead of %CC_PATH%\find.exe on next runs.
echo ;%PATH%; | find /c /i ";%CC_PATH%;" >nul || set PATH=%PATH%;%CC_PATH%

%MK_EXEC% RM="del /f /q" MV="move /y" MATCH="find /i /c" BIN=rc4.exe DLDFLAGS="" %*
::%MK_EXEC% MV="copy /y" MATCH="find /i /c" BIN=rc4.exe DLDFLAGS="" %*
::%MK_EXEC% BIN=rc4.exe %*
