@echo off
cc  winmain.c  -o winmain -l gdi32 -l xinput
winmain
