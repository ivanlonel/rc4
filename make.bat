@echo off

REM Compiler path. May contain the 'make' application and other utilities as well.
set CC_PATH=C:\mingw-w64\x86_64-6.2.0-release-posix-seh-rt_v5-rev1\mingw64\bin
::set CC_PATH=C:\mingw-w64\x86_64-5.4.0-release-posix-seh-rt_v5-rev0\mingw64\bin
::set CC_PATH=C:\mingw-w64\x86_64-4.9.3-release-posix-seh-rt_v4-rev1\mingw64\bin
::set CC_PATH=C:\Dev-Cpp\MinGW64\bin
::set CC_PATH=C:\CodeBlocks\MinGW\bin
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


::set OUTPUT="/Ox /ML" 	REM /ML is the compiler's default option. Causes the compiler to place the library name LIBC.lib into the .obj file so that the linker will use LIBC.lib to resolve external symbols. LIBC.lib does not provide multithread support.
::set OUTPUT="/Ox /MT" 	REM /MT Causes the application to use the multithread, static version of the run-time library. Defines _MT and causes the compiler to place the library name LIBCMT.lib into the .obj file so that the linker will use LIBCMT.lib to resolve external symbols.
::set OUTPUT="/Ox /MD" 	REM /MD Causes the application to use the multithread-specific and DLL-specific version of the run-time library. Defines _MT and _DLL and causes the compiler to place the library name MSVCRT.lib into the .obj file. Applications compiled with this option are statically linked to MSVCRT.lib. This library provides a layer of code that enables the linker to resolve external references. The actual working code is contained in MSVCR<versionnumber>.DLL, which must be available at run time to applications linked with MSVCRT.lib.
::set OUTPUT="/Z7 /MLd"	REM /MLd Defines _DEBUG and causes the compiler to place the library name LIBCD.lib into the .obj file so that the linker will use LIBCD.lib to resolve external symbols. LIBCD.lib does not provide multithread support.
::set OUTPUT="/Z7 /MTd"	REM /MTd Defines _DEBUG and _MT. This option also causes the compiler to place the library name LIBCMTD.lib into the .obj file so that the linker will use LIBCMTD.lib to resolve external symbols.
::set OUTPUT="/Z7 /MDd"	REM /MDd Defines _DEBUG, _MT, and _DLL and causes the application to use the debug multithread-specific and DLL-specific version of the run-time library. It also causes the compiler to place the library name MSVCRTD.lib into the .obj file.

::cl src\*.c /Iinclude /Fo.o\ /Febin\rc4.exe /Za %OUTPUT% /MP /nologo /Wall /D_CRT_SECURE_NO_WARNINGS /GA /Gw /GL /analyze:stacksize 65536