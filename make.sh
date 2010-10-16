#!/bin/bash
i486-mingw32-gcc -c -I. -o dialog.o dialog.c || exit -1
i486-mingw32-gcc -c -I. -o main.o main.c || exit -1
i486-mingw32-windres notepad.rc rc.o || exit -1
i486-mingw32-gcc -o notepad.exe dialog.o main.o rc.o -g -mwindows -lshlwapi || exit -1
