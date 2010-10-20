/* Lev Panov, 2057/2, October 2010 */

#define WINVER 0x0500

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <stdbool.h>

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
void SetFileName(const char *FileName)
{
    lstrcpy(Globals.FileName, FileName);
    Globals.FileTitle[0] = 0;
    GetFileTitle(FileName, Globals.FileTitle, sizeof(Globals.FileTitle));
}

/***********************************************************************
 *
 *           NOTEPAD_SetParams
 */
static void NOTEPAD_SetParams(void)
{
    int base_length, dx, dy;

    base_length = (GetSystemMetrics(SM_CXSCREEN) > GetSystemMetrics(SM_CYSCREEN)) ?
                   GetSystemMetrics(SM_CYSCREEN) : GetSystemMetrics(SM_CXSCREEN);

    dx = base_length * 0.95;
    dy = dx * 3 / 4;
    SetRect(&main_rect, 0, 0, dx, dy);

    Globals.isWrapLongLines = true;
}

/***********************************************************************
 *
 *           NOTEPAD_OnCreate
 */
static bool NOTEPAD_OnCreate(HWND hWnd, CREATESTRUCT *cs)
{
    HDC hDC;
    TEXTMETRIC metric;

    hDC = GetDC(hWnd);
    SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));

    GetTextMetrics(hDC, &metric);
    Globals.CharWidth = metric.tmAveCharWidth;
    Globals.CharHeight = metric.tmHeight + metric.tmExternalLeading;

    CreateCaret(hWnd, NULL, 0, Globals.CharHeight);
    SetCaretPos(0, 0);
    ShowCaret(hWnd);

    ReleaseDC(hWnd, hDC);

    return true;
}

/***********************************************************************
 *
 *           NOTEPAD_OnSize
 */
static void NOTEPAD_OnSize(HWND hWnd, UINT State, INT W, INT H)
{
    InvalidateRect(hWnd, NULL, false);
}

/***********************************************************************
 *
 *           NOTEPAD_OnMenuCommand
 *
 *  All handling of main menu events
 */
void NOTEPAD_OnMenuCommand(HWND hwnd, int Id, HWND hwndCtl, uint codeNotify)
{
    switch (Id)
    {
        case CMD_NEW:     DIALOG_FileNew(); break;
        case CMD_OPEN:    DIALOG_FileOpen(); break;
        case CMD_SAVE:    DIALOG_FileSave(); break;
        case CMD_SAVE_AS: DIALOG_FileSaveAs(); break;
        case CMD_EXIT:    DIALOG_FileExit(); break;

        case CMD_WRAP:    DIALOG_EditWrap(); break;

        default:          break;
    }
}

/***********************************************************************
 *
 *           NOTEPAD_OnDropFiles
 */
static void NOTEPAD_OnDropFiles(HWND hWnd, HDROP hDrop)
{
    char FileName[MAX_PATH];
    //HANDLE hDrop = (HANDLE)wParam;

    DragQueryFile(hDrop, 0, FileName, ARRAY_SIZE(FileName));
    DragFinish(hDrop);
    DoOpenFile(FileName);
}

/***********************************************************************
 *
 *           NOTEPAD_OnPaint
 */
static void NOTEPAD_OnPaint(HWND hWnd)
{
    HDC hDC;
    PAINTSTRUCT ps;
    RECT rc;

    GetClientRect(hWnd, &rc);

    hDC = BeginPaint(hWnd, &ps);

    FillRect(hDC, &rc, GetStockObject(WHITE_BRUSH));

    if (Globals.FileName[0] != '\0') {
        TextItem *a;
        int i = 0;
        for (a = Globals.TextList.first; ; a = a->next) {
            TextOut(hDC, 0, Globals.CharHeight * i++, a->str.data, a->str.len);
            if (a == Globals.TextList.last) {
                break;
            }
        }
    }

    EndPaint(hWnd, &ps);
}

/***********************************************************************
 *
 *           NOTEPAD_OnVScroll
 */
static void NOTEPAD_OnVScroll(HWND hWnd, HWND hWndCtl, uint Code, int Pos)
{

}

/***********************************************************************
 *
 *           NOTEPAD_OnHScroll
 */
static VOID NOTEPAD_OnHScroll(HWND hWnd, HWND hWndCtl, uint Code, int Pos )
{

}

/***********************************************************************
 *
 *           NOTEPAD_OnChar
 */
void NOTEPAD_OnChar(HWND hWnd, char Ch, int cRepeat)
{

}

/***********************************************************************
 *
 *           NOTEPAD_OnKeyDown
 */
static void NOTEPAD_OnKeyDown(HWND hWnd, uint Vkey, bool Down, int Repeat, uint flags )
{

}

/***********************************************************************
 *
 *           NOTEPAD_OnClose
 */
static void NOTEPAD_OnClose(HWND hWnd)
{
    if (DoCloseFile()) {
        DestroyWindow(hWnd);
    }
}

/***********************************************************************
 *
 *           NOTEPAD_OnDestroy
 */
static void NOTEPAD_OnDestroy(HWND hWnd)
{
    DestroyCaret();

    PostQuitMessage(0);
}


/***********************************************************************
 * Data Initialization
 */
static void NOTEPAD_InitData(void)
{
    char *p = Globals.Filter;
    static const char txt_files[] = { '*','.','t','x','t',0 };
    static const char all_files[] = { '*','.','*',0 };

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
            MF_BYCOMMAND | (Globals.isWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
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
        HANDLE_MSG(hWnd, WM_CREATE, NOTEPAD_OnCreate);
        HANDLE_MSG(hWnd, WM_SIZE, NOTEPAD_OnSize);
        HANDLE_MSG(hWnd, WM_COMMAND, NOTEPAD_OnMenuCommand);
        HANDLE_MSG(hWnd, WM_DROPFILES, NOTEPAD_OnDropFiles);

        HANDLE_MSG(hWnd, WM_PAINT, NOTEPAD_OnPaint);
        HANDLE_MSG(hWnd, WM_VSCROLL, NOTEPAD_OnVScroll);
        HANDLE_MSG(hWnd, WM_HSCROLL, NOTEPAD_OnHScroll);
        HANDLE_MSG(hWnd, WM_KEYDOWN, NOTEPAD_OnKeyDown);
        HANDLE_MSG(hWnd, WM_CHAR, NOTEPAD_OnChar);

        HANDLE_MSG(hWnd, WM_CLOSE, NOTEPAD_OnClose);
        HANDLE_MSG(hWnd, WM_DESTROY, NOTEPAD_OnDestroy);

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

#if 0
        case WM_CREATE:
            break;

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
            char FileName[MAX_PATH];
            HANDLE hDrop = (HANDLE) wParam;

            DragQueryFile(hDrop, 0, FileName, ARRAY_SIZE(FileName));
            DragFinish(hDrop);
            DoOpenFile(FileName);
            break;
        }

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
#endif
}

static int AlertFileDoesNotExist(const char *FileName)
{
   int Result;
   char Message[MAX_STRING_LEN];
   char Resource[MAX_STRING_LEN];

   LoadString(Globals.hInstance, STRING_DOESNOTEXIST, Resource, ARRAY_SIZE(Resource));
   wsprintf(Message, Resource, FileName);

   LoadString(Globals.hInstance, STRING_ERROR, Resource, ARRAY_SIZE(Resource));

   Result = MessageBox(Globals.hMainWnd, Message, Resource,
                         MB_ICONEXCLAMATION | MB_YESNOCANCEL);

   return(Result);
}

static void HandleCommandLine(char *cmdline)
{
    if (*cmdline)
    {
        /* file name is passed in the command line */
        if (FileExists(cmdline))
        {
            DoOpenFile(cmdline);
            InvalidateRect(Globals.hMainWnd, NULL, false);
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
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE prev, char *cmdline, int show)
{
    MSG msg;
    WNDCLASSEX wc;
    HMONITOR monitor;
    MONITORINFO info;
    int x, y;
    static const char className[] = {'N','o','t','e','p','a','d',0};
    static const char winName[]   = {'N','o','t','e','p','a','d',0};

    ZeroMemory(&Globals, sizeof(Globals));
    Globals.hInstance       = hInstance;
    NOTEPAD_SetParams();

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = NOTEPAD_WndProc;
    wc.hInstance     = Globals.hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE(MAIN_MENU);
    wc.lpszClassName = className;

    if (!RegisterClassEx(&wc)) return false;

    /* Setup windows */

    monitor = MonitorFromRect(&main_rect, MONITOR_DEFAULTTOPRIMARY);
    info.cbSize = sizeof(info);
    GetMonitorInfo(monitor, &info);

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
    DragAcceptFiles(Globals.hMainWnd, true);

    HandleCommandLine(cmdline);

    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
