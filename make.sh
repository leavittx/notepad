i486-mingw32-gcc -c -I. -o dialog.o dialog.c
i486-mingw32-gcc -c -I. -o main.o main.c
i486-mingw32-windres notepad.rc rc.o
i486-mingw32-gcc -o notepad.exe dialog.o main.o rc.o -g -mwindows -lshlwapi
