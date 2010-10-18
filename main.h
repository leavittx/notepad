/* Lev Panov, 2057/2, October 2010 */

#ifndef MAIN_H
#define MAIN_H

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#include "notepad_res.h"

#define MAX_STRING_LEN 255

typedef struct
{
  HANDLE  hInstance;
  HWND    hMainWnd;
  BOOL    bWrapLongLines;
  CHAR    szFileName[MAX_PATH];
  CHAR    szFileTitle[MAX_PATH];
  CHAR    szFilter[2 * MAX_STRING_LEN + 100];
} NOTEPAD_GLOBALS;

extern NOTEPAD_GLOBALS Globals;

void SetFileName(LPCSTR szFileName);

#endif // MAIN_H
