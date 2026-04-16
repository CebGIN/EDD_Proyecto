@echo off
set "CC=g++"
set "CFLAGS=-std=c++17 -Wall -Wextra -Isrc -Isrc/external"
set "SRCS=src/main.cpp src/csvReader/csvReader.cpp src/mafiaFamily/mafiaTree.cpp"
set "OUT_DIR=bin"
set "TARGET=%OUT_DIR%\mafia_system.exe"

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

echo Compiling project...
%CC% %CFLAGS% %SRCS% -o %TARGET%

if %ERRORLEVEL% equ 0 (
    echo Build successful! Executable is at %TARGET%
) else (
    echo Build failed with error code %ERRORLEVEL%
    pause
)
