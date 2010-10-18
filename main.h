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
  HWND    hEdit;
  HFONT   hFont; /* Font used by the edit control */
  LOGFONT lfFont;
  BOOL    bWrapLongLines;
  CHAR    szFileName[MAX_PATH];
  CHAR    szFileTitle[MAX_PATH];
  CHAR    szFilter[2 * MAX_STRING_LEN + 100];
  BOOL    bOfnIsOpenDialog;
  INT     iMarginTop;
  INT     iMarginBottom;
  INT     iMarginLeft;
  INT     iMarginRight;
  CHAR    szHeader[MAX_PATH];
  CHAR    szFooter[MAX_PATH];
} NOTEPAD_GLOBALS;

extern NOTEPAD_GLOBALS Globals;

VOID SetFileName(LPCSTR szFileName);

#endif // MAIN_H
