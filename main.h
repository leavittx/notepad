#ifndef MAIN_H
#define MAIN_H

#define ARRAY_SIZE(a) sizeof(a)/sizeof((a)[0])

#include "notepad_res.h"

#define MAX_STRING_LEN      255

/* Values are indexes of the items in the Encoding combobox. */
typedef enum
{
    ENCODING_AUTO    = -1,
    ENCODING_ANSI    =  0,
    ENCODING_UTF16LE =  1,
    ENCODING_UTF16BE =  2,
    ENCODING_UTF8    =  3
} ENCODING;

#define MIN_ENCODING   0
#define MAX_ENCODING   3

typedef struct
{
  HANDLE   hInstance;
  HWND     hMainWnd;
  HWND     hFindReplaceDlg;
  HWND     hEdit;
  HFONT    hFont; /* Font used by the edit control */
  LOGFONTW lfFont;
  BOOL     bWrapLongLines;
  WCHAR    szFindText[MAX_PATH];
  WCHAR    szReplaceText[MAX_PATH];
  WCHAR    szFileName[MAX_PATH];
  WCHAR    szFileTitle[MAX_PATH];
  ENCODING encFile;
  WCHAR    szFilter[2 * MAX_STRING_LEN + 100];
  ENCODING encOfnCombo;  /* Encoding selected in IDC_OFN_ENCCOMBO */
  BOOL     bOfnIsOpenDialog;
  INT      iMarginTop;
  INT      iMarginBottom;
  INT      iMarginLeft;
  INT      iMarginRight;
  WCHAR    szHeader[MAX_PATH];
  WCHAR    szFooter[MAX_PATH];

  FINDREPLACEW find;
  FINDREPLACEW lastFind;
  HGLOBAL hDevMode; /* printer mode */
  HGLOBAL hDevNames; /* printer names */
} NOTEPAD_GLOBALS;

extern NOTEPAD_GLOBALS Globals;

VOID SetFileNameAndEncoding(LPCWSTR szFileName, ENCODING enc);
void NOTEPAD_DoFind(FINDREPLACEW *fr);
DWORD get_dpi(void);

#endif // MAIN_H
