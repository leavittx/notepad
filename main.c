#define WINVER 0x0500
#include <windows.h>
#include <shlwapi.h>
//#include <winuser.h>

#include "main.h"
#include "dialog.h"
#include "notepad_res.h"

NOTEPAD_GLOBALS Globals;
static RECT main_rect;

/***********************************************************************
 *
 *           SetFileNameAndEncoding
 *
 *  Sets global file name and encoding (which is used to preselect original
 *  encoding in Save As dialog, and when saving without using the Save As
 *  dialog).
 */
VOID SetFileNameAndEncoding(LPCWSTR szFileName, ENCODING enc)
{
    lstrcpyW(Globals.szFileName, szFileName);
    Globals.szFileTitle[0] = 0;
    GetFileTitleW(szFileName, Globals.szFileTitle, sizeof(Globals.szFileTitle));
    Globals.encFile = enc;
}

/******************************************************************************
 *      get_dpi
 *
 * Get the dpi from registry HKCC\Software\Fonts\LogPixels.
 */
DWORD get_dpi(void)
{
    static const WCHAR dpi_key_name[] = {'S','o','f','t','w','a','r','e','\\','F','o','n','t','s','\0'};
    static const WCHAR dpi_value_name[] = {'L','o','g','P','i','x','e','l','s','\0'};
    DWORD dpi = 96;
    HKEY hkey;

    if (RegOpenKeyW(HKEY_CURRENT_CONFIG, dpi_key_name, &hkey) == ERROR_SUCCESS)
    {
        DWORD type, size, new_dpi;

        size = sizeof(new_dpi);
        if(RegQueryValueExW(hkey, dpi_value_name, NULL, &type, (LPBYTE)&new_dpi, &size) == ERROR_SUCCESS)
        {
            if(type == REG_DWORD && new_dpi != 0)
                dpi = new_dpi;
        }
        RegCloseKey(hkey);
    }
    return dpi;
}

/***********************************************************************
 *
 *           NOTEPAD_LoadSettingFromRegistry
 *
 *  Load setting from registry HKCU\Software\Microsoft\Notepad.
 */
static VOID NOTEPAD_LoadSettingFromRegistry(void)
{
    static const WCHAR systemW[] = { 'S','y','s','t','e','m','\0' };
    HKEY hkey;
    INT base_length, dx, dy;

    base_length = (GetSystemMetrics(SM_CXSCREEN) > GetSystemMetrics(SM_CYSCREEN))?
        GetSystemMetrics(SM_CYSCREEN) : GetSystemMetrics(SM_CXSCREEN);

    dx = base_length * .95;
    dy = dx * 3 / 4;
    SetRect( &main_rect, 0, 0, dx, dy );

    Globals.bWrapLongLines  = TRUE;
    Globals.iMarginTop = 2500;
    Globals.iMarginBottom = 2500;
    Globals.iMarginLeft = 2000;
    Globals.iMarginRight = 2000;

    Globals.lfFont.lfHeight         = -12;
    Globals.lfFont.lfWidth          = 0;
    Globals.lfFont.lfEscapement     = 0;
    Globals.lfFont.lfOrientation    = 0;
    Globals.lfFont.lfWeight         = FW_REGULAR;
    Globals.lfFont.lfItalic         = FALSE;
    Globals.lfFont.lfUnderline      = FALSE;
    Globals.lfFont.lfStrikeOut      = FALSE;
    Globals.lfFont.lfCharSet        = DEFAULT_CHARSET;
    Globals.lfFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    Globals.lfFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    Globals.lfFont.lfQuality        = DEFAULT_QUALITY;
    Globals.lfFont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
    lstrcpyW(Globals.lfFont.lfFaceName, systemW);
}

/***********************************************************************
 *
 *           NOTEPAD_MenuCommand
 *
 *  All handling of main menu events
 */
static int NOTEPAD_MenuCommand(WPARAM wParam)
{
    switch (wParam)
    {
    case CMD_NEW:               DIALOG_FileNew(); break;
    case CMD_OPEN:              DIALOG_FileOpen(); break;
    case CMD_SAVE:              DIALOG_FileSave(); break;
    case CMD_SAVE_AS:           DIALOG_FileSaveAs(); break;
    case CMD_EXIT:              DIALOG_FileExit(); break;

    case CMD_CUT:              DIALOG_EditCut(); break;
    case CMD_COPY:             DIALOG_EditCopy(); break;
    case CMD_PASTE:            DIALOG_EditPaste(); break;
    case CMD_DELETE:           DIALOG_EditDelete(); break;
    case CMD_SELECT_ALL:       DIALOG_EditSelectAll(); break;

    case CMD_WRAP:             DIALOG_EditWrap(); break;
    case CMD_FONT:             DIALOG_SelectFont(); break;

    case CMD_HELP_ABOUT_NOTEPAD: DIALOG_About(); break;

    default:
        break;
    }
   return 0;
}

/***********************************************************************
 * Data Initialization
 */
static VOID NOTEPAD_InitData(VOID)
{
    LPWSTR p = Globals.szFilter;
    static const WCHAR txt_files[] = { '*','.','t','x','t',0 };
    static const WCHAR all_files[] = { '*','.','*',0 };

    LoadStringW(Globals.hInstance, STRING_TEXT_FILES_TXT, p, MAX_STRING_LEN);
    p += lstrlenW(p) + 1;
    lstrcpyW(p, txt_files);
    p += lstrlenW(p) + 1;
    LoadStringW(Globals.hInstance, STRING_ALL_FILES, p, MAX_STRING_LEN);
    p += lstrlenW(p) + 1;
    lstrcpyW(p, all_files);
    p += lstrlenW(p) + 1;
    *p = '\0';
    Globals.hDevMode = NULL;
    Globals.hDevNames = NULL;

    CheckMenuItem(GetMenu(Globals.hMainWnd), CMD_WRAP,
            MF_BYCOMMAND | (Globals.bWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
}

/***********************************************************************
 * Enable/disable items on the menu based on control state
 */
static VOID NOTEPAD_InitMenuPopup(HMENU menu, int index)
{
    int enable;

    EnableMenuItem(menu, CMD_PASTE,
        IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED);
    enable = SendMessageW(Globals.hEdit, EM_GETSEL, 0, 0);
    enable = (HIWORD(enable) == LOWORD(enable)) ? MF_GRAYED : MF_ENABLED;
    EnableMenuItem(menu, CMD_CUT, enable);
    EnableMenuItem(menu, CMD_COPY, enable);
    EnableMenuItem(menu, CMD_DELETE, enable);

    EnableMenuItem(menu, CMD_SELECT_ALL,
        GetWindowTextLengthW(Globals.hEdit) ? MF_ENABLED : MF_GRAYED);
}

static LPWSTR NOTEPAD_StrRStr(LPWSTR pszSource, LPWSTR pszLast, LPWSTR pszSrch)
{
    int len = lstrlenW(pszSrch);
    pszLast--;
    while (pszLast >= pszSource)
    {
        if (StrCmpNW(pszLast, pszSrch, len) == 0)
            return pszLast;
        pszLast--;
    }
    return NULL;
}

/***********************************************************************
 *
 *           NOTEPAD_WndProc
 */
static LRESULT WINAPI NOTEPAD_WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                               LPARAM lParam)
{
    switch (msg) {

    case WM_CREATE:
    {
        static const WCHAR editW[] = { 'e','d','i','t',0 };
        DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
                        ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
        RECT rc;
        GetClientRect(hWnd, &rc);

        if (!Globals.bWrapLongLines) dwStyle |= WS_HSCROLL | ES_AUTOHSCROLL;

        Globals.hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, editW, NULL,
                             dwStyle, 0, 0, rc.right, rc.bottom, hWnd,
                             NULL, Globals.hInstance, NULL);

        Globals.hFont = CreateFontIndirectW(&Globals.lfFont);
        SendMessageW(Globals.hEdit, WM_SETFONT, (WPARAM)Globals.hFont, FALSE);
        SendMessageW(Globals.hEdit, EM_LIMITTEXT, 0, 0);
        break;
    }

    case WM_COMMAND:
        NOTEPAD_MenuCommand(LOWORD(wParam));
        break;

    case WM_DESTROYCLIPBOARD:
        /*MessageBoxW(Globals.hMainWnd, "Empty clipboard", "Debug", MB_ICONEXCLAMATION);*/
        break;

    case WM_CLOSE:
        if (DoCloseFile()) {
            DestroyWindow(hWnd);
        }
        break;

    case WM_QUERYENDSESSION:
        if (DoCloseFile()) {
            return 1;
        }
        break;

    case WM_DESTROY:
        //NOTEPAD_SaveSettingToRegistry();

        PostQuitMessage(0);
        break;

    case WM_SIZE:
        SetWindowPos(Globals.hEdit, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam),
                     SWP_NOOWNERZORDER | SWP_NOZORDER);
        break;

    case WM_SETFOCUS:
        SetFocus(Globals.hEdit);
        break;

    case WM_DROPFILES:
    {
        WCHAR szFileName[MAX_PATH];
        HANDLE hDrop = (HANDLE) wParam;

        DragQueryFileW(hDrop, 0, szFileName, ARRAY_SIZE(szFileName));
        DragFinish(hDrop);
        DoOpenFile(szFileName, ENCODING_AUTO);
        break;
    }

    case WM_INITMENUPOPUP:
        NOTEPAD_InitMenuPopup((HMENU)wParam, lParam);
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static int AlertFileDoesNotExist(LPCWSTR szFileName)
{
   int nResult;
   WCHAR szMessage[MAX_STRING_LEN];
   WCHAR szResource[MAX_STRING_LEN];

   LoadStringW(Globals.hInstance, STRING_DOESNOTEXIST, szResource, ARRAY_SIZE(szResource));
   wsprintfW(szMessage, szResource, szFileName);

   LoadStringW(Globals.hInstance, STRING_ERROR, szResource, ARRAY_SIZE(szResource));

   nResult = MessageBoxW(Globals.hMainWnd, szMessage, szResource,
                         MB_ICONEXCLAMATION | MB_YESNOCANCEL);

   return(nResult);
}

static void HandleCommandLine(LPWSTR cmdline)
{
    WCHAR delimiter;

    /* skip white space */
    while (*cmdline == ' ') cmdline++;

    /* skip executable name */
    delimiter = (*cmdline == '"' ? '"' : ' ');

    if (*cmdline == delimiter) cmdline++;

    while (*cmdline && *cmdline != delimiter) cmdline++;

    if (*cmdline == delimiter) cmdline++;

    while (*cmdline == ' ' || *cmdline == '-' || *cmdline == '/')
    {
        cmdline++;
    }

    if (*cmdline)
    {
        /* file name is passed in the command line */
        LPCWSTR file_name;
        BOOL file_exists;
        WCHAR buf[MAX_PATH];

        if (cmdline[0] == '"')
        {
            WCHAR* wc;
            cmdline++;
            wc=cmdline;
            /* Note: Double-quotes are not allowed in Windows filenames */
            while (*wc && *wc != '"') wc++;
            /* On Windows notepad ignores further arguments too */
            *wc = 0;
        }

        if (FileExists(cmdline))
        {
            file_exists = TRUE;
            file_name = cmdline;
        }
        else
        {
            static const WCHAR txtW[] = { '.','t','x','t',0 };

            /* try to find file with ".txt" extension */
            // strchrW - no such func
            if (wcschr(PathFindFileNameW(cmdline), '.'))
            {
                file_exists = FALSE;
                file_name = cmdline;
            }
            else
            {
                lstrcpynW(buf, cmdline, MAX_PATH - lstrlenW(txtW) - 1);
                lstrcatW(buf, txtW);
                file_name = buf;
                file_exists = FileExists(buf);
            }
        }

        if (file_exists)
        {
            DoOpenFile(file_name, ENCODING_AUTO);
            InvalidateRect(Globals.hMainWnd, NULL, FALSE);
        }
        else
        {
            switch (AlertFileDoesNotExist(file_name)) {
            case IDYES:
                SetFileNameAndEncoding(file_name, ENCODING_ANSI);
                UpdateWindowCaption();
                break;

            case IDNO:
                break;

            case IDCANCEL:
                DestroyWindow(Globals.hMainWnd);
                break;
            }
        }
     }
}

/***********************************************************************
 *
 *           WinMain
 */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR cmdline, int show)
{
    MSG msg;
    HACCEL hAccel;
    WNDCLASSEXW class;
    HMONITOR monitor;
    MONITORINFO info;
    INT x, y;
    static const WCHAR className[] = {'N','o','t','e','p','a','d',0};
    static const WCHAR winName[]   = {'N','o','t','e','p','a','d',0};

    ZeroMemory(&Globals, sizeof(Globals));
    Globals.hInstance       = hInstance;
    NOTEPAD_LoadSettingFromRegistry();

    ZeroMemory(&class, sizeof(class));
    class.cbSize        = sizeof(class);
    class.lpfnWndProc   = NOTEPAD_WndProc;
    class.hInstance     = Globals.hInstance;
    class.hIcon         = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    class.hIconSm       = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    class.hCursor       = LoadCursorW(0, (LPCWSTR)IDC_ARROW);
    class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    class.lpszMenuName  = MAKEINTRESOURCEW(MAIN_MENU);
    class.lpszClassName = className;

    if (!RegisterClassExW(&class)) return FALSE;

    /* Setup windows */

    monitor = MonitorFromRect( &main_rect, MONITOR_DEFAULTTOPRIMARY );
    info.cbSize = sizeof(info);
    GetMonitorInfoW( monitor, &info );

    x = main_rect.left;
    y = main_rect.top;
    if (main_rect.left >= info.rcWork.right ||
        main_rect.top >= info.rcWork.bottom ||
        main_rect.right < info.rcWork.left ||
        main_rect.bottom < info.rcWork.top)
        x = y = CW_USEDEFAULT;

    Globals.hMainWnd =
        CreateWindowW(className, winName, WS_OVERLAPPEDWINDOW, x, y,
                      main_rect.right - main_rect.left, main_rect.bottom - main_rect.top,
                      NULL, NULL, Globals.hInstance, NULL);
    if (!Globals.hMainWnd)
    {
        ShowLastError();
        ExitProcess(1);
    }

    NOTEPAD_InitData();
    DIALOG_FileNew();

    ShowWindow(Globals.hMainWnd, show);
    UpdateWindow(Globals.hMainWnd);
    DragAcceptFiles(Globals.hMainWnd, TRUE);

    HandleCommandLine(GetCommandLineW());

    while (GetMessageW(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return msg.wParam;
}
