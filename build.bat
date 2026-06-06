@echo off
setlocal EnableExtensions EnableDelayedExpansion

if /i "%~1"=="test" goto :test

REM Windows build - sources listed in build\game_sources.txt

where g++ >nul 2>nul
if errorlevel 1 (
    echo Error: g++ not found. Install MinGW-w64 or MSYS2 and add g++ to PATH.
    echo   MSYS2: pacman -S mingw-w64-x86_64-gcc
    exit /b 1
)

if not exist third_party\json\single_include\nlohmann\json.hpp (
    echo Error: missing third_party/json. Run:
    echo   git submodule update --init --recursive
    exit /b 1
)

if not exist build mkdir build
if not exist bin mkdir bin
if not exist data mkdir data

set CXXFLAGS=-std=c++11 -Wall -Wextra -O2 -Iinclude -I.

if exist build\objects.rsp del /q build\objects.rsp
echo Compiling...
for /f "usebackq delims=" %%s in ("build\game_sources.txt") do (
    if not "%%s"=="" (
        g++ %CXXFLAGS% -c src\%%s -o build\%%~ns.o || exit /b 1
        >>build\objects.rsp echo build/%%~ns.o
    )
)

del /q build\test_runner.o build\gametester.o build\utils_test.o build\utils_clang.o build\savegame.o 2>nul

echo Linking game...
g++ %CXXFLAGS% @build\objects.rsp -o bin\game.exe || exit /b 1
echo Build successful! Run: bin\game.exe
goto :eof

:test
if not exist build mkdir build
if not exist bin mkdir bin
set CXXFLAGS=-std=c++11 -Wall -Wextra -O2 -Iinclude -I.
echo Compiling tests...
g++ %CXXFLAGS% -c src\gametester.cpp    -o build\gametester.o    || exit /b 1
g++ %CXXFLAGS% -c src\test_runner.cpp   -o build\test_runner.o   || exit /b 1
g++ %CXXFLAGS% -c src\monster.cpp       -o build\monster.o       || exit /b 1
g++ %CXXFLAGS% -c src\player.cpp        -o build\player.o        || exit /b 1
g++ %CXXFLAGS% -c src\trap.cpp          -o build\trap.o          || exit /b 1
g++ %CXXFLAGS% -c src\item.cpp          -o build\item.o          || exit /b 1
g++ %CXXFLAGS% -c src\types.cpp         -o build\types.o         || exit /b 1
g++ %CXXFLAGS% -c src\utils.cpp         -o build\utils.o         || exit /b 1
g++ %CXXFLAGS% -c src\balance.cpp       -o build\balance.o       || exit /b 1
g++ %CXXFLAGS% -c src\mapgenerator.cpp  -o build\mapgenerator.o  || exit /b 1
g++ %CXXFLAGS% -c src\room.cpp          -o build\room.o          || exit /b 1
g++ %CXXFLAGS% -c src\battlesystem.cpp  -o build\battlesystem.o  || exit /b 1
echo Linking tests...
g++ %CXXFLAGS% build\gametester.o build\test_runner.o build\monster.o build\player.o ^
  build\trap.o build\item.o build\types.o build\utils.o build\balance.o ^
  build\mapgenerator.o build\room.o build\battlesystem.o -o bin\gametest.exe || exit /b 1
echo Running tests...
bin\gametest.exe
