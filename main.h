/* Lev Panov, 2057/2, October 2010 */

#ifndef MAIN_H
#define MAIN_H

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef unsigned int uint;

#include "edit.h"
#include "notepad_res.h"

#define MAX_STRING_LEN 255

typedef struct {
    HANDLE hInstance;
    HWND   hMainWnd;
    bool   isWrapLongLines;
    char   FileName[MAX_PATH];
    char   FileTitle[MAX_PATH];
    char   Filter[2 * MAX_STRING_LEN + 100];
    Text   TextList;
    int    CharWidth, CharHeight;
} NOTEPAD_GLOBALS;

extern NOTEPAD_GLOBALS Globals;

void SetFileName(const char *FileName);

#endif // MAIN_H
