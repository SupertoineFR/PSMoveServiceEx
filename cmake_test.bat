@echo off
cd /d "C:\Program Files\CMake\bin"
cmake -G "Visual Studio 16 2019" --version 2>&1 | head -5