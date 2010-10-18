/* Lev Panov, 2057/2, October 2010 */

#include <windows.h>
#include <shlwapi.h> // for wnsprintf()

#include "main.h"
#include "dialog.h"

void ShowLastError(void)
{
    DWORD error = GetLastError();
    if (error != NO_ERROR)
    {
        LPSTR lpMsgBuf;
        CHAR szTitle[MAX_STRING_LEN];

        LoadString(Globals.hInstance, STRING_ERROR, szTitle, ARRAY_SIZE(szTitle));
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, error, 0, (LPSTR)&lpMsgBuf, 0, NULL);
        MessageBox(NULL, lpMsgBuf, szTitle, MB_OK | MB_ICONERROR);
        LocalFree(lpMsgBuf);
    }
}

/**
 * Sets the caption of the main window according to Globals.szFileTitle:
 *    Untitled - Notepad        if no file is open
 *    filename - Notepad        if a file is given
 */
void UpdateWindowCaption(void)
{
  CHAR szCaption[MAX_STRING_LEN];
  CHAR szNotepad[MAX_STRING_LEN];
  static const CHAR hyphenW[] = { ' ','-',' ',0 };

  if (Globals.szFileTitle[0] != '\0')
      lstrcpy(szCaption, Globals.szFileTitle);
  else
      LoadString(Globals.hInstance, STRING_UNTITLED, szCaption, ARRAY_SIZE(szCaption));

  LoadString(Globals.hInstance, STRING_NOTEPAD, szNotepad, ARRAY_SIZE(szNotepad));
  lstrcat(szCaption, hyphenW);
  lstrcat(szCaption, szNotepad);

  SetWindowText(Globals.hMainWnd, szCaption);
}

int DIALOG_StringMsgBox(HWND hParent, int formatId, LPCSTR szString, DWORD dwFlags)
{
   CHAR szMessage[MAX_STRING_LEN];
   CHAR szResource[MAX_STRING_LEN];

   /* Load and format szMessage */
   LoadString(Globals.hInstance, formatId, szResource, ARRAY_SIZE(szResource));
   wnsprintf(szMessage, ARRAY_SIZE(szMessage), szResource, szString);

   /* Load szCaption */
   if ((dwFlags & MB_ICONMASK) == MB_ICONEXCLAMATION)
     LoadString(Globals.hInstance, STRING_ERROR,  szResource, ARRAY_SIZE(szResource));
   else
     LoadString(Globals.hInstance, STRING_NOTEPAD,  szResource, ARRAY_SIZE(szResource));

   /* Display Modal Dialog */
   if (hParent == NULL)
     hParent = Globals.hMainWnd;
   return MessageBox(hParent, szMessage, szResource, dwFlags);
}

static void AlertFileNotFound(LPCSTR szFileName)
{
   DIALOG_StringMsgBox(NULL, STRING_NOTFOUND, szFileName, MB_ICONEXCLAMATION|MB_OK);
}

static int AlertFileNotSaved(LPCSTR szFileName)
{
   CHAR szUntitled[MAX_STRING_LEN];

   LoadString(Globals.hInstance, STRING_UNTITLED, szUntitled, ARRAY_SIZE(szUntitled));
   return DIALOG_StringMsgBox(NULL, STRING_NOTSAVED, szFileName[0] ? szFileName : szUntitled,
     MB_ICONQUESTION|MB_YESNOCANCEL);
}

/**
 * Returns:
 *   TRUE  - if file exists
 *   FALSE - if file does not exist
 */
BOOL FileExists(LPCSTR szFilename)
{
   WIN32_FIND_DATA entry;
   HANDLE hFile;

   hFile = FindFirstFile(szFilename, &entry);
   FindClose(hFile);

   return (hFile != INVALID_HANDLE_VALUE);
}

typedef enum
{
    SAVED_OK,
    SAVE_FAILED,
    SHOW_SAVEAS_DIALOG
} SAVE_STATUS;

/* szFileName is the filename to save under.
 *
 * If the function succeeds, it returns SAVED_OK.
 * If the function fails, it returns SAVE_FAILED.
 */
static SAVE_STATUS DoSaveFile(LPCSTR szFileName)
{
    return SAVED_OK;
}

/**
 * Returns:
 *   TRUE  - User agreed to close (both save/don't save)
 *   FALSE - User cancelled close by selecting "Cancel"
 */
BOOL DoCloseFile(void)
{
    int nResult;
    static const CHAR empty_strW[] = { 0 };

    if (0/*ismodified*/)
    {
        /* prompt user to save changes */
        nResult = AlertFileNotSaved(Globals.szFileName);
        switch (nResult)
        {
            case IDYES:
                return DIALOG_FileSave();

            case IDNO:
                break;

            case IDCANCEL:
                return(FALSE);

            default:
                return(FALSE);
        }
    }

    SetFileName(empty_strW);

    UpdateWindowCaption();
    return(TRUE);
}

void DoOpenFile(LPCSTR szFileName)
{
    /* Close any files and prompt to save changes */
    if (!DoCloseFile())
        return;

    // Open the file

    SetFileName(szFileName);
    UpdateWindowCaption();
}

void DIALOG_FileNew(void)
{
    static const CHAR empty_strW[] = { 0 };

    /* Close any files and prompt to save changes */
    if (DoCloseFile()) {
        SetWindowText(Globals.hMainWnd, empty_strW);
        UpdateWindowCaption();
    }
}

void DIALOG_FileOpen(void)
{
    OPENFILENAME openfilename;
    CHAR szPath[MAX_PATH];
    static const CHAR szDefaultExt[] = { 't','x','t',0 };
    static const CHAR txt_files[] = { '*','.','t','x','t',0 };

    ZeroMemory(&openfilename, sizeof(openfilename));

    lstrcpy(szPath, txt_files);

    openfilename.lStructSize       = sizeof(openfilename);
    openfilename.hwndOwner         = Globals.hMainWnd;
    openfilename.hInstance         = Globals.hInstance;
    openfilename.lpstrFilter       = Globals.szFilter;
    openfilename.lpstrFile         = szPath;
    openfilename.nMaxFile          = ARRAY_SIZE(szPath);
    openfilename.Flags = OFN_ENABLETEMPLATE | OFN_EXPLORER |
                         OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                         OFN_HIDEREADONLY | OFN_ENABLESIZING;
    openfilename.lpstrDefExt       = szDefaultExt;

    if (GetOpenFileName(&openfilename))
        DoOpenFile(openfilename.lpstrFile);
}

/* Return FALSE to cancel close */
BOOL DIALOG_FileSave(void)
{
    if (Globals.szFileName[0] == '\0')
        return DIALOG_FileSaveAs();
    else
    {
        switch (DoSaveFile(Globals.szFileName))
        {
            case SAVED_OK:
                return TRUE;
            case SHOW_SAVEAS_DIALOG:
                return DIALOG_FileSaveAs();
            default:
                return FALSE;
        }
    }
}

BOOL DIALOG_FileSaveAs(void)
{
    OPENFILENAME saveas;
    CHAR szPath[MAX_PATH];
    static const CHAR szDefaultExt[] = { 't','x','t',0 };
    static const CHAR txt_files[] = { '*','.','t','x','t',0 };

    ZeroMemory(&saveas, sizeof(saveas));

    lstrcpy(szPath, txt_files);

    saveas.lStructSize       = sizeof(OPENFILENAMEW);
    saveas.hwndOwner         = Globals.hMainWnd;
    saveas.hInstance         = Globals.hInstance;
    saveas.lpstrFilter       = Globals.szFilter;
    saveas.lpstrFile         = szPath;
    saveas.nMaxFile          = ARRAY_SIZE(szPath);
    saveas.Flags          = OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_EXPLORER |
                            OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT |
                            OFN_HIDEREADONLY | OFN_ENABLESIZING;
    saveas.lpstrDefExt       = szDefaultExt;

retry:
    if (!GetSaveFileName(&saveas))
        return FALSE;

    switch (DoSaveFile(szPath))
    {
        case SAVED_OK:
            SetFileName(szPath);
            UpdateWindowCaption();
            return TRUE;

        case SHOW_SAVEAS_DIALOG:
            goto retry;

        default:
            return FALSE;
    }
}

void DIALOG_FileExit(void)
{
    PostMessage(Globals.hMainWnd, WM_CLOSE, 0, 0l);
}

void DIALOG_EditWrap(void)
{
    Globals.bWrapLongLines = !Globals.bWrapLongLines;
    CheckMenuItem(GetMenu(Globals.hMainWnd), CMD_WRAP,
        MF_BYCOMMAND | (Globals.bWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
}
