#!/bin/bash
i486-mingw32-gcc -Wall -c -I. -o dialog.o dialog.c || exit -1
i486-mingw32-gcc -Wall -c -I. -o edit.o edit.c || exit -1
i486-mingw32-gcc -Wall -c -I. -o main.o main.c || exit -1
i486-mingw32-windres notepad.rc rc.o || exit -1
i486-mingw32-gcc -Wall -o notepad.exe edit.o dialog.o main.o rc.o -mwindows -lshlwapi || exit -1
