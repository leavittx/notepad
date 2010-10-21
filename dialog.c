/* Lev Panov, 2057/2, October 2010 */

#include <windows.h>
#include <shlwapi.h> // for wnsprintf()
#include <stdio.h>

#include "main.h"
#include "dialog.h"

void ShowLastError(void)
{
    DWORD error = GetLastError();
    if (error != NO_ERROR)
    {
        char *lpMsgBuf;
        char Title[MAX_STRING_LEN];

        LoadString(Globals.hInstance, STRING_ERROR, Title, ARRAY_SIZE(Title));
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, error, 0, (char *)&lpMsgBuf, 0, NULL);
        MessageBox(NULL, lpMsgBuf, Title, MB_OK | MB_ICONERROR);
        LocalFree(lpMsgBuf);
    }
}

/**
 * Sets the caption of the main window according to Globals.FileTitle:
 *    Untitled - Notepad        if no file is open
 *    filename - Notepad        if a file is given
 */
void UpdateWindowCaption(void)
{
  char Caption[MAX_STRING_LEN];
  char Notepad[MAX_STRING_LEN];
  static const char hyphenW[] = { ' ','-',' ',0 };

  if (Globals.FileTitle[0] != '\0')
      lstrcpy(Caption, Globals.FileTitle);
  else
      LoadString(Globals.hInstance, STRING_UNTITLED, Caption, ARRAY_SIZE(Caption));

  LoadString(Globals.hInstance, STRING_NOTEPAD, Notepad, ARRAY_SIZE(Notepad));
  lstrcat(Caption, hyphenW);
  lstrcat(Caption, Notepad);

  SetWindowText(Globals.hMainWnd, Caption);
}

int DIALOG_StringMsgBox(HWND hParent, int formatId, const char *String, DWORD dwFlags)
{
   char Message[MAX_STRING_LEN];
   char Resource[MAX_STRING_LEN];

   /* Load and format  Message */
   LoadString(Globals.hInstance, formatId, Resource, ARRAY_SIZE(Resource));
   wnsprintf(Message, ARRAY_SIZE(Message), Resource, String);

   /* Load  Caption */
   if ((dwFlags & MB_ICONMASK) == MB_ICONEXCLAMATION)
     LoadString(Globals.hInstance, STRING_ERROR,  Resource, ARRAY_SIZE(Resource));
   else
     LoadString(Globals.hInstance, STRING_NOTEPAD,  Resource, ARRAY_SIZE(Resource));

   /* Display Modal Dialog */
   if (hParent == NULL)
     hParent = Globals.hMainWnd;
   return MessageBox(hParent, Message, Resource, dwFlags);
}

static void AlertFileNotFound(const char *FileName)
{
   DIALOG_StringMsgBox(NULL, STRING_NOTFOUND, FileName, MB_ICONEXCLAMATION|MB_OK);
}

static int AlertFileNotSaved(const char *FileName)
{
   char Untitled[MAX_STRING_LEN];

   LoadString(Globals.hInstance, STRING_UNTITLED, Untitled, ARRAY_SIZE(Untitled));
   return DIALOG_StringMsgBox(NULL, STRING_NOTSAVED, FileName[0] ? FileName :  Untitled,
     MB_ICONQUESTION|MB_YESNOCANCEL);
}

/**
 * Returns:
 *   true  - if file exists
 *   false - if file does not exist
 */
bool FileExists(const char *Filename)
{
    FILE *f;

    f = fopen(Filename, "rt");
    if (f == NULL) {
        return false;
    }
    else {
        fclose(f);
        return true;
    }
}

typedef enum {
    SAVED_OK,
    SAVE_FAILED,
    SHOW_SAVEAS_DIALOG
} SAVE_STATUS;

/* FileName is the filename to save under.
 *
 * If the function succeeds, it returns SAVED_OK.
 * If the function fails, it returns SAVE_FAILED.
 */
static SAVE_STATUS DoSaveFile(const char *FileName)
{
    return SAVED_OK;
}

/**
 * Returns:
 *   true  - User agreed to close (both save/don't save)
 *   false - User cancelled close by selecting "Cancel"
 */
bool DoCloseFile(void)
{
    int Result;
    static const char empty_str[] = { 0 };

    if (0/*ismodified*/) {
        /* prompt user to save changes */
        Result = AlertFileNotSaved(Globals.FileName);
        switch (Result) {
            case IDYES:     return DIALOG_FileSave();
            case IDNO:      break;
            case IDCANCEL:  return(false);
            default:        return(false);
        }
    }

    SetFileName(empty_str);

    UpdateWindowCaption();
    return(true);
}

void DoOpenFile(const char *FileName)
{
    FILE *f;
    //int size;
    int curstrlen = 0;

    /* Close any files and prompt to save changes */
    if (!DoCloseFile())
        return;

    f = fopen(FileName, "rb");
    if (f == NULL) {
        AlertFileNotFound(FileName);
        return;
    }

    //fseek(f, 0, SEEK_END);
    //size = ftell(f);
    //if (size > MAX_FILE_SIZE)
        //AlertFileTooBig();
    //rewind(f);

    Globals.TextList.LongestStringLength = 0;
    while (1) {
        int c = fgetc(f);
        if (c == '\n') {
            fseek(f, -curstrlen-1, SEEK_CUR);
            EDIT_AddTextItem(f, curstrlen);
            fgetc(f); // Read '\n'
            if (curstrlen > Globals.TextList.LongestStringLength)
                Globals.TextList.LongestStringLength = curstrlen;
            curstrlen = 0;
        }
        else if (c == EOF) {
            fseek(f, -curstrlen, SEEK_CUR);
            EDIT_AddTextItem(f, curstrlen);
            if (curstrlen > Globals.TextList.LongestStringLength)
                Globals.TextList.LongestStringLength = curstrlen;
            break;
        }
        else {
            curstrlen++;
        }
    }

    for (TextItem *a = Globals.TextList.first; ; a = a->next) {
        a->noffsets = 0;
        if (a == Globals.TextList.last)
            break;
    }
    EDIT_CountOffsets();

#ifdef DEBUG
    for (TextItem *a = Globals.TextList.first; ; a = a->next) {
        fputs(a->str.data, stdout);
        fputc('\n', stdout);
        if (a == Globals.TextList.last)
            break;
    }
#endif

    SetFileName(FileName);
    UpdateWindowCaption();

    SendMessage(Globals.hMainWnd, WM_SIZE, 0, MAKELONG(Globals.W, Globals.H));
}

void DIALOG_FileNew(void)
{
    static const char empty_str[] = { 0 };

    /* Close any files and prompt to save changes */
    if (DoCloseFile()) {
        SetWindowText(Globals.hMainWnd, empty_str);
        UpdateWindowCaption();
    }
}

void DIALOG_FileOpen(void)
{
    OPENFILENAME openfilename;
    char Path[MAX_PATH];
    static const char DefaultExt[] = { 't','x','t',0 };
    static const char txt_files[] = { '*','.','t','x','t',0 };

    ZeroMemory(&openfilename, sizeof(openfilename));

    lstrcpy(Path, txt_files);

    openfilename.lStructSize       = sizeof(openfilename);
    openfilename.hwndOwner         = Globals.hMainWnd;
    openfilename.hInstance         = Globals.hInstance;
    openfilename.lpstrFilter       = Globals.Filter;
    openfilename.lpstrFile         = Path;
    openfilename.nMaxFile          = ARRAY_SIZE(Path);
    openfilename.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                         OFN_HIDEREADONLY | OFN_ENABLESIZING;
    openfilename.lpstrDefExt       = DefaultExt;

    if (GetOpenFileName(&openfilename))
        DoOpenFile(openfilename.lpstrFile);
}

/* Return false to cancel close */
bool DIALOG_FileSave(void)
{
    if (Globals.FileName[0] == '\0')
        return DIALOG_FileSaveAs();
    else
    {
        switch (DoSaveFile(Globals.FileName))
        {
            case SAVED_OK:           return true;
            case SHOW_SAVEAS_DIALOG: return DIALOG_FileSaveAs();
            default:                 return false;
        }
    }
}

bool DIALOG_FileSaveAs(void)
{
    OPENFILENAME saveas;
    char Path[MAX_PATH];
    static const char DefaultExt[] = { 't','x','t',0 };
    static const char txt_files[] = { '*','.','t','x','t',0 };

    ZeroMemory(&saveas, sizeof(saveas));

    lstrcpy(Path, txt_files);

    saveas.lStructSize       = sizeof(OPENFILENAMEW);
    saveas.hwndOwner         = Globals.hMainWnd;
    saveas.hInstance         = Globals.hInstance;
    saveas.lpstrFilter       = Globals.Filter;
    saveas.lpstrFile         = Path;
    saveas.nMaxFile          = ARRAY_SIZE(Path);
    saveas.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT |
                   OFN_HIDEREADONLY | OFN_ENABLESIZING;
    saveas.lpstrDefExt       = DefaultExt;

retry:
    if (!GetSaveFileName(&saveas))
        return false;

    switch (DoSaveFile(Path))
    {
        case SAVED_OK:
            SetFileName(Path);
            UpdateWindowCaption();
            return true;

        case SHOW_SAVEAS_DIALOG:
            goto retry;

        default:
            return false;
    }
}

void DIALOG_FileExit(void)
{
    PostMessage(Globals.hMainWnd, WM_CLOSE, 0, 0l);
}

void DIALOG_EditWrap(void)
{
    Globals.isWrapLongLines = !Globals.isWrapLongLines;
    CheckMenuItem(GetMenu(Globals.hMainWnd), CMD_WRAP,
        MF_BYCOMMAND | (Globals.isWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
    SendMessage(Globals.hMainWnd, WM_SIZE, 0, MAKELONG(Globals.W, Globals.H));
}
