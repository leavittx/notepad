/* Lev Panov, 2057/2, October 2010 */

#ifndef MAIN_H
#define MAIN_H

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef unsigned int uint;

#include "edit.h"
#include "notepad_res.h"

#define MAX_STRING_LEN 255

#define LF '\n'
#define CR '\r'

typedef enum {
    EOL_LF,
    EOL_CRLF
} EOL_TYPE;

typedef struct {
    HANDLE hInstance;
    HWND   hMainWnd;
    int    W, H;
    bool   isWrapLongLines;
    char   FileName[MAX_PATH];
    char   FileTitle[MAX_PATH];
    char   Filter[MAX_STRING_LEN];
    EOL_TYPE EOL_type;
    Text   TextList;
    int    CharW, CharH;
} NOTEPAD_GLOBALS;

extern NOTEPAD_GLOBALS Globals;

void SetFileName(const char *FileName);

#endif // MAIN_H
