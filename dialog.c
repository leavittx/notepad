/* Lev Panov, 2057/2, October 2010 */

#include <windows.h>
#include <shlwapi.h> // for wnsprintf()

#include "main.h"
#include "dialog.h"

#define SPACES_IN_TAB 8
#define PRINT_LEN_MAX 500

VOID ShowLastError(void)
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
    int lenW;
    CHAR* textW;
    HANDLE hFile;
    DWORD dwNumWrite;
    PVOID pBytes;
    DWORD size;

    /* lenW includes the byte-order mark, but not the \0. */
    lenW = GetWindowTextLength(Globals.hEdit) + 1;
    textW = HeapAlloc(GetProcessHeap(), 0, (lenW+1) * sizeof(CHAR));
    if (!textW)
    {
        ShowLastError();
        return SAVE_FAILED;
    }
    //textW[0] = (CHAR) 0xfeff;
    lenW = GetWindowText(Globals.hEdit, textW+1, lenW) + 1;

    ////////////////////////////
    //size = WideCharToMultiByte(CP_ACP, 0, textW+1, lenW-1, NULL, 0, NULL, NULL);
    size = lenW;
    pBytes = HeapAlloc(GetProcessHeap(), 0, size);
    if (!pBytes)
    {
        ShowLastError();
        HeapFree(GetProcessHeap(), 0, textW);
        return SAVE_FAILED;
    }
    //WideCharToMultiByte(CP_ACP, 0, textW+1, lenW-1, pBytes, size, NULL, NULL);
    memcpy(pBytes, textW+1, size);

    HeapFree(GetProcessHeap(), 0, textW);
    /////////////////////////////

    hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        ShowLastError();
        HeapFree(GetProcessHeap(), 0, pBytes);
        return SAVE_FAILED;
    }
    if (!WriteFile(hFile, pBytes, size, &dwNumWrite, NULL))
    {
        ShowLastError();
        CloseHandle(hFile);
        HeapFree(GetProcessHeap(), 0, pBytes);
        return SAVE_FAILED;
    }
    SetEndOfFile(hFile);
    CloseHandle(hFile);
    HeapFree(GetProcessHeap(), 0, pBytes);

    SendMessage(Globals.hEdit, EM_SETMODIFY, FALSE, 0);
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

    nResult=GetWindowTextLength(Globals.hEdit);
    if (SendMessage(Globals.hEdit, EM_GETMODIFY, 0, 0) &&
        (nResult || Globals.szFileName[0]))
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
    HANDLE hFile;
    LPSTR pTemp;
    DWORD size;
    DWORD dwNumRead;
    int lenW;
    CHAR* textW;
    int i;

    /* Close any files and prompt to save changes */
    if (!DoCloseFile())
        return;

    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        AlertFileNotFound(szFileName);
        return;
    }

    size = GetFileSize(hFile, NULL);
    if (size == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        ShowLastError();
        return;
    }

    /* Extra memory for (CHAR)'\0'-termination. */
    pTemp = HeapAlloc(GetProcessHeap(), 0, size+2);
    if (!pTemp)
    {
        CloseHandle(hFile);
        ShowLastError();
        return;
    }

    if (!ReadFile(hFile, pTemp, size, &dwNumRead, NULL))
    {
        CloseHandle(hFile);
        HeapFree(GetProcessHeap(), 0, pTemp);
        ShowLastError();
        return;
    }

    CloseHandle(hFile);

    size = dwNumRead;

    ////////////////////////////
    //lenW = MultiByteToWideChar(CP_ACP, 0, pTemp, size, NULL, 0);
    lenW = size;
    textW = HeapAlloc(GetProcessHeap(), 0, (lenW+1) * sizeof(CHAR));
    if (!textW)
    {
        ShowLastError();
        HeapFree(GetProcessHeap(), 0, pTemp);
        return;
    }
    //MultiByteToWideChar(CP_ACP, 0, pTemp, size, textW, lenW);
    memcpy(textW, pTemp, size);

    HeapFree(GetProcessHeap(), 0, pTemp);
    /////////////////////////////

    /* Replace '\0's with spaces. Other than creating a custom control that
     * can deal with '\0' characters, it's the best that can be done.
     */
    for (i = 0; i < lenW; i++)
        if (textW[i] == '\0')
            textW[i] = ' ';
    textW[lenW] = '\0';

    /*if (lenW >= 1 && textW[0] == 0xfeff)
        SetWindowText(Globals.hEdit, textW+1);
    else*/
        SetWindowText(Globals.hEdit, textW);

    HeapFree(GetProcessHeap(), 0, textW);

    SendMessage(Globals.hEdit, EM_SETMODIFY, FALSE, 0);
    SetFocus(Globals.hEdit);

    SetFileName(szFileName);
    UpdateWindowCaption();
}

VOID DIALOG_FileNew(VOID)
{
    static const CHAR empty_strW[] = { 0 };

    /* Close any files and prompt to save changes */
    if (DoCloseFile()) {
        SetWindowText(Globals.hEdit, empty_strW);
        SetFocus(Globals.hEdit);
    }
}

VOID DIALOG_FileOpen(VOID)
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

    Globals.bOfnIsOpenDialog = TRUE;

    if (GetOpenFileName(&openfilename))
        DoOpenFile(openfilename.lpstrFile);
}

/* Return FALSE to cancel close */
BOOL DIALOG_FileSave(VOID)
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

BOOL DIALOG_FileSaveAs(VOID)
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

    Globals.bOfnIsOpenDialog = FALSE;

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

typedef struct {
    LPSTR mptr;
    LPSTR mend;
    LPSTR lptr;
    DWORD len;
} TEXTINFO, *LPTEXTINFO;

static int notepad_print_header(HDC hdc, RECT *rc, BOOL dopage, BOOL header, int page, LPSTR text)
{
    SIZE szMetric;

    if (*text)
    {
        /* Write the header or footer */
        GetTextExtentPoint32(hdc, text, lstrlen(text), &szMetric);
        if (dopage)
            ExtTextOut(hdc, (rc->left + rc->right - szMetric.cx) / 2,
                        header ? rc->top : rc->bottom - szMetric.cy,
                        ETO_CLIPPED, rc, text, lstrlen(text), NULL);
        return 1;
    }
    return 0;
}

static CHAR *expand_header_vars(CHAR *pattern, int page)
{
    int length = 0;
    int i;
    BOOL inside = FALSE;
    CHAR *buffer = NULL;

    for (i = 0; pattern[i]; i++)
    {
        if (inside)
        {
            if (pattern[i] == '&')
                length++;
            else if (pattern[i] == 'p')
                length += 11;
            inside = FALSE;
        }
        else if (pattern[i] == '&')
            inside = TRUE;
        else
            length++;
    }

    buffer = HeapAlloc(GetProcessHeap(), 0, (length + 1) * sizeof(CHAR));
    if (buffer)
    {
        int j = 0;
        inside = FALSE;
        for (i = 0; pattern[i]; i++)
        {
            if (inside)
            {
                if (pattern[i] == '&')
                    buffer[j++] = '&';
                else if (pattern[i] == 'p')
                {
                    static const CHAR percent_dW[] = {'%','d',0};
                    j += wnsprintf(&buffer[j], 11, percent_dW, page);
                }
                inside = FALSE;
            }
            else if (pattern[i] == '&')
                inside = TRUE;
            else
                buffer[j++] = pattern[i];
        }
        buffer[j++] = 0;
    }
    return buffer;
}

static BOOL notepad_print_page(HDC hdc, RECT *rc, BOOL dopage, int page, LPTEXTINFO tInfo)
{
    int b, y;
    TEXTMETRIC tm;
    SIZE szMetrics;
    CHAR *footer_text = NULL;

    footer_text = expand_header_vars(Globals.szFooter, page);
    if (footer_text == NULL)
        return FALSE;

    if (dopage)
    {
        if (StartPage(hdc) <= 0)
        {
            static const CHAR failedW[] = { 'S','t','a','r','t','P','a','g','e',' ','f','a','i','l','e','d',0 };
            static const CHAR errorW[] = { 'P','r','i','n','t',' ','E','r','r','o','r',0 };
            MessageBox(Globals.hMainWnd, failedW, errorW, MB_ICONEXCLAMATION);
            HeapFree(GetProcessHeap(), 0, footer_text);
            return FALSE;
        }
    }

    GetTextMetrics(hdc, &tm);
    y = rc->top + notepad_print_header(hdc, rc, dopage, TRUE, page, Globals.szFileName) * tm.tmHeight;
    b = rc->bottom - 2 * notepad_print_header(hdc, rc, FALSE, FALSE, page, footer_text) * tm.tmHeight;

    do {
        INT m, n;

        if (!tInfo->len)
        {
            /* find the end of the line */
            while (tInfo->mptr < tInfo->mend && *tInfo->mptr != '\n' && *tInfo->mptr != '\r')
            {
                if (*tInfo->mptr == '\t')
                {
                    /* replace tabs with spaces */
                    for (m = 0; m < SPACES_IN_TAB; m++)
                    {
                        if (tInfo->len < PRINT_LEN_MAX)
                            tInfo->lptr[tInfo->len++] = ' ';
                        else if (Globals.bWrapLongLines)
                            break;
                    }
                }
                else if (tInfo->len < PRINT_LEN_MAX)
                    tInfo->lptr[tInfo->len++] = *tInfo->mptr;

                if (tInfo->len >= PRINT_LEN_MAX && Globals.bWrapLongLines)
                     break;

                tInfo->mptr++;
            }
        }

        /* Find out how much we should print if line wrapping is enabled */
        if (Globals.bWrapLongLines)
        {
            GetTextExtentExPoint(hdc, tInfo->lptr, tInfo->len, rc->right - rc->left, &n, NULL, &szMetrics);
            if (n < tInfo->len && tInfo->lptr[n] != ' ')
            {
                m = n;
                /* Don't wrap words unless it's a single word over the entire line */
                while (m  && tInfo->lptr[m] != ' ') m--;
                if (m > 0) n = m + 1;
            }
        }
        else
            n = tInfo->len;

        if (dopage)
            ExtTextOut(hdc, rc->left, y, ETO_CLIPPED, rc, tInfo->lptr, n, NULL);

        tInfo->len -= n;

        if (tInfo->len)
        {
            memcpy(tInfo->lptr, tInfo->lptr + n, tInfo->len * sizeof(CHAR));
            y += tm.tmHeight + tm.tmExternalLeading;
        }
        else
        {
            /* find the next line */
            while (tInfo->mptr < tInfo->mend && y < b && (*tInfo->mptr == '\n' || *tInfo->mptr == '\r'))
            {
                if (*tInfo->mptr == '\n')
                    y += tm.tmHeight + tm.tmExternalLeading;
                tInfo->mptr++;
            }
        }
    } while (tInfo->mptr < tInfo->mend && y < b);

    notepad_print_header(hdc, rc, dopage, FALSE, page, footer_text);
    if (dopage)
    {
        EndPage(hdc);
    }
    HeapFree(GetProcessHeap(), 0, footer_text);
    return TRUE;
}

VOID DIALOG_FileExit(VOID)
{
    PostMessage(Globals.hMainWnd, WM_CLOSE, 0, 0l);
}

VOID DIALOG_EditWrap(VOID)
{
    BOOL modify = FALSE;
    static const CHAR editW[] = { 'e','d','i','t',0 };
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
                    ES_AUTOVSCROLL | ES_MULTILINE;
    RECT rc;
    DWORD size;
    LPSTR pTemp;

    size = GetWindowTextLength(Globals.hEdit) + 1;
    pTemp = HeapAlloc(GetProcessHeap(), 0, size * sizeof(CHAR));
    if (!pTemp)
    {
        ShowLastError();
        return;
    }
    GetWindowText(Globals.hEdit, pTemp, size);
    modify = SendMessage(Globals.hEdit, EM_GETMODIFY, 0, 0);
    DestroyWindow(Globals.hEdit);
    GetClientRect(Globals.hMainWnd, &rc);
    if( Globals.bWrapLongLines ) dwStyle |= WS_HSCROLL | ES_AUTOHSCROLL;
    Globals.hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, editW, NULL, dwStyle,
                         0, 0, rc.right, rc.bottom, Globals.hMainWnd,
                         NULL, Globals.hInstance, NULL);
    SendMessage(Globals.hEdit, WM_SETFONT, (WPARAM)Globals.hFont, FALSE);
    SetWindowText(Globals.hEdit, pTemp);
    SendMessage(Globals.hEdit, EM_SETMODIFY, modify, 0);
    SetFocus(Globals.hEdit);
    HeapFree(GetProcessHeap(), 0, pTemp);

    Globals.bWrapLongLines = !Globals.bWrapLongLines;
    CheckMenuItem(GetMenu(Globals.hMainWnd), CMD_WRAP,
        MF_BYCOMMAND | (Globals.bWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
}

VOID DIALOG_SelectFont(VOID)
{
    CHOOSEFONT cf;
    LOGFONT lf=Globals.lfFont;

    ZeroMemory( &cf, sizeof(cf) );
    cf.lStructSize=sizeof(cf);
    cf.hwndOwner=Globals.hMainWnd;
    cf.lpLogFont=&lf;
    cf.Flags=CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

    if( ChooseFont(&cf) )
    {
        HFONT currfont=Globals.hFont;

        Globals.hFont=CreateFontIndirect( &lf );
        Globals.lfFont=lf;
        SendMessage( Globals.hEdit, WM_SETFONT, (WPARAM)Globals.hFont, TRUE );
        if( currfont!=NULL )
            DeleteObject( currfont );
    }
}
