@echo off
gcc -g -O0 winmain.c -o winmain.exe -luser32 -lgdi32 -lxinput
winmain