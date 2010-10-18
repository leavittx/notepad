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
void SetFileName(LPCSTR szFileName)
{
    lstrcpy(Globals.szFileName, szFileName);
    Globals.szFileTitle[0] = 0;
    GetFileTitle(szFileName, Globals.szFileTitle, sizeof(Globals.szFileTitle));
}

/***********************************************************************
 *
 *           NOTEPAD_SetParams
 */
static void NOTEPAD_SetParams(void)
{
    static const CHAR systemW[] = { 'S','y','s','t','e','m','\0' };
    INT base_length, dx, dy;

    base_length = (GetSystemMetrics(SM_CXSCREEN) > GetSystemMetrics(SM_CYSCREEN))?
        GetSystemMetrics(SM_CYSCREEN) : GetSystemMetrics(SM_CXSCREEN);

    dx = base_length * .95;
    dy = dx * 3 / 4;
    SetRect( &main_rect, 0, 0, dx, dy );

    Globals.bWrapLongLines = TRUE;
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

        case CMD_WRAP:              DIALOG_EditWrap(); break;

        default:
            break;
    }
   return 0;
}

/***********************************************************************
 * Data Initialization
 */
static void NOTEPAD_InitData(void)
{
    LPSTR p = Globals.szFilter;
    static const CHAR txt_files[] = { '*','.','t','x','t',0 };
    static const CHAR all_files[] = { '*','.','*',0 };

    LoadString(Globals.hInstance, STRING_TEXT_FILES_TXT, p, MAX_STRING_LEN);
    p += lstrlen(p) + 1;
    lstrcpy(p, txt_files);
    p += lstrlen(p) + 1;
    LoadString(Globals.hInstance, STRING_ALL_FILES, p, MAX_STRING_LEN);
    p += lstrlen(p) + 1;
    lstrcpy(p, all_files);
    p += lstrlen(p) + 1;
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
    switch (msg)
    {
        case WM_CREATE:
        {
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
            break;

        case WM_DROPFILES:
        {
            CHAR szFileName[MAX_PATH];
            HANDLE hDrop = (HANDLE) wParam;

            DragQueryFile(hDrop, 0, szFileName, ARRAY_SIZE(szFileName));
            DragFinish(hDrop);
            DoOpenFile(szFileName);
            break;
        }

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static int AlertFileDoesNotExist(LPCSTR szFileName)
{
   int nResult;
   CHAR szMessage[MAX_STRING_LEN];
   CHAR szResource[MAX_STRING_LEN];

   LoadString(Globals.hInstance, STRING_DOESNOTEXIST, szResource, ARRAY_SIZE(szResource));
   wsprintf(szMessage, szResource, szFileName);

   LoadString(Globals.hInstance, STRING_ERROR, szResource, ARRAY_SIZE(szResource));

   nResult = MessageBox(Globals.hMainWnd, szMessage, szResource,
                         MB_ICONEXCLAMATION | MB_YESNOCANCEL);

   return(nResult);
}

static void HandleCommandLine(LPSTR cmdline)
{
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
            switch (AlertFileDoesNotExist(cmdline))
            {
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
}

/***********************************************************************
 *
 *           WinMain
 */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR cmdline, int show)
{
    MSG msg;
    WNDCLASSEX wc;
    HMONITOR monitor;
    MONITORINFO info;
    INT x, y;
    static const CHAR className[] = {'N','o','t','e','p','a','d',0};
    static const CHAR winName[]   = {'N','o','t','e','p','a','d',0};

    ZeroMemory(&Globals, sizeof(Globals));
    Globals.hInstance       = hInstance;
    NOTEPAD_SetParams();

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = NOTEPAD_WndProc;
    wc.hInstance     = Globals.hInstance;
    wc.hIcon         = LoadIcon(NULL, (LPCSTR)IDI_APPLICATION);
    wc.hIconSm       = LoadIcon(NULL, (LPCSTR)IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE(MAIN_MENU);
    wc.lpszClassName = className;

    if (!RegisterClassEx(&wc)) return FALSE;

    /* Setup windows */

    monitor = MonitorFromRect( &main_rect, MONITOR_DEFAULTTOPRIMARY );
    info.cbSize = sizeof(info);
    GetMonitorInfo( monitor, &info );

    x = main_rect.left;
    y = main_rect.top;
    if (main_rect.left >= info.rcWork.right ||
        main_rect.top >= info.rcWork.bottom ||
        main_rect.right < info.rcWork.left ||
        main_rect.bottom < info.rcWork.top)
        x = y = CW_USEDEFAULT;

    Globals.hMainWnd =
        CreateWindow(className, winName, WS_OVERLAPPEDWINDOW, x, y,
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

    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
