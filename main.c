/* Lev Panov, 2057/2, October 2010 */

#define WINVER 0x0500

#include <windows.h>
#include <shlwapi.h>

#include "main.h"
#include "dialog.h"
#include "notepad_res.h"

NOTEPAD_GLOBALS Globals;
static RECT main_rect;

/***********************************************************************
 *
 *           SetFileName
 *
 *  Sets global file name.
 */
VOID SetFileName(LPCWSTR szFileName)
{
    lstrcpyW(Globals.szFileName, szFileName);
    Globals.szFileTitle[0] = 0;
    GetFileTitleW(szFileName, Globals.szFileTitle, sizeof(Globals.szFileTitle));
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

    CheckMenuItem(GetMenu(Globals.hMainWnd), CMD_WRAP,
            MF_BYCOMMAND | (Globals.bWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
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
        DoOpenFile(szFileName);
        break;
    }

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

static void HandleCommandLine(LPSTR cmdline)
{
#if 0
    if (*cmdline)
    {
        /* file name is passed in the command line */
        if (FileExists(cmdline))
        {
            DoOpenFile(cmdline);
            InvalidateRect(Globals.hMainWnd, NULL, FALSE);
        }
        else
        {
            switch (AlertFileDoesNotExist(cmdline)) {
            case IDYES:
                SetFileName(cmdline);
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
#endif
}

/***********************************************************************
 *
 *           WinMain
 */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR cmdline, int show)
{
    MSG msg;
    HACCEL hAccel;
    WNDCLASSEXW wc;
    HMONITOR monitor;
    MONITORINFO info;
    INT x, y;
    static const WCHAR className[] = {'N','o','t','e','p','a','d',0};
    static const WCHAR winName[]   = {'N','o','t','e','p','a','d',0};

    ZeroMemory(&Globals, sizeof(Globals));
    Globals.hInstance       = hInstance;
    NOTEPAD_LoadSettingFromRegistry();

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = NOTEPAD_WndProc;
    wc.hInstance     = Globals.hInstance;
    wc.hIcon         = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    wc.hIconSm       = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    wc.hCursor       = LoadCursorW(0, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = MAKEINTRESOURCEW(MAIN_MENU);
    wc.lpszClassName = className;

    if (!RegisterClassExW(&wc)) return FALSE;

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

    HandleCommandLine(cmdline);

    while (GetMessageW(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return msg.wParam;
}
