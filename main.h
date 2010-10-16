/* Lev Panov, 2057/2, October 2010 */

#ifndef MAIN_H
#define MAIN_H

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#include "notepad_res.h"

#define MAX_STRING_LEN 255

typedef struct
{
  HANDLE   hInstance;
  HWND     hMainWnd;
  HWND     hEdit;
  HFONT    hFont; /* Font used by the edit control */
  LOGFONTW lfFont;
  BOOL     bWrapLongLines;
  WCHAR    szFileName[MAX_PATH];
  WCHAR    szFileTitle[MAX_PATH];
  WCHAR    szFilter[2 * MAX_STRING_LEN + 100];
  BOOL     bOfnIsOpenDialog;
  INT      iMarginTop;
  INT      iMarginBottom;
  INT      iMarginLeft;
  INT      iMarginRight;
  WCHAR    szHeader[MAX_PATH];
  WCHAR    szFooter[MAX_PATH];
} NOTEPAD_GLOBALS;

extern NOTEPAD_GLOBALS Globals;

VOID SetFileNameAndEncoding(LPCWSTR szFileName);
void NOTEPAD_DoFind(FINDREPLACEW *fr);
DWORD get_dpi(void);

#endif // MAIN_H
