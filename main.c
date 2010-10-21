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

    Globals.W = dx;
    Globals.H = dy;

    Globals.isWrapLongLines = true;
}

/***********************************************************************
 *
 *           NOTEPAD_OnCreate
 */
static bool NOTEPAD_OnCreate(HWND hWnd, CREATESTRUCT *cs)
{
    HDC hDC;
    TEXTMETRIC text_metric;

    hDC = GetDC(hWnd);
    SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));

    GetTextMetrics(hDC, &text_metric);
    Globals.CharW = text_metric.tmAveCharWidth;
    Globals.CharH = text_metric.tmHeight + text_metric.tmExternalLeading;

    CreateCaret(hWnd, NULL, 0, Globals.CharH);
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
    SCROLLINFO scroll_info;

    Globals.W = W;
    Globals.H = H;

    if (Globals.FileName[0] == '\0' ||
            W == 0 || H == 0) {
        return;
    }

    EDIT_CountOffsets();

    // Vertical scroll
    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask = SIF_PAGE | SIF_RANGE;
    scroll_info.nMin = 0;
    scroll_info.nMax = Globals.TextList.nDrawLines - 1;

    printf("scroll max: %i\n",  scroll_info.nMax);

    scroll_info.nPage = H / Globals.CharH;
    SetScrollInfo(hWnd, SB_VERT, &scroll_info, true);

    // Horizontal scroll
    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask = SIF_PAGE | SIF_RANGE;
    scroll_info.nMin = 0;
    if (Globals.isWrapLongLines)
        scroll_info.nMax = W / Globals.CharW;
    else
        scroll_info.nMax = Globals.TextList.LongestStringLength;

    printf("scroll max: %i\n",  scroll_info.nMax);

    scroll_info.nPage = W / Globals.CharW; //+1
    SetScrollInfo(hWnd, SB_HORZ, &scroll_info, true);

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
    switch (Id) {
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
    SCROLLINFO scroll_info;
    int vert_pos, horiz_pos, paint_beg, paint_end;
    int noffset = 0,
        maxlen = Globals.W / Globals.CharW,
        drawlen, drawoffset, drawremain;
    int x = 0, y = 0;
    int i;

    GetClientRect(hWnd, &rc);
    hDC = BeginPaint(hWnd, &ps);
    FillRect(hDC, &rc, GetStockObject(WHITE_BRUSH));

    // TODO
    if (Globals.FileName[0] == '\0') {
        EndPaint(hWnd, &ps);
        return;
    }

    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask  = SIF_POS;
    GetScrollInfo(hWnd, SB_VERT, &scroll_info);
    vert_pos = scroll_info.nPos;

    GetScrollInfo(hWnd, SB_HORZ, &scroll_info);
    horiz_pos = scroll_info.nPos;

    printf("scroll: %i %i\n", vert_pos, horiz_pos);

    paint_beg = max(0, vert_pos + ps.rcPaint.top / Globals.CharH);
    paint_end = min(Globals.TextList.nDrawLines - 1,
                    vert_pos + ps.rcPaint.bottom / Globals.CharH);

    printf("paint: %i %i\n", paint_beg, paint_end);

    TextItem *a;
    for (a = Globals.TextList.first;
         a->drawnums[0] < paint_beg && a != Globals.TextList.last;
         a = a->next);
    if (a == Globals.TextList.last) {
        if (a->drawnums[0] + a->noffsets - 1 >= paint_beg) { // -1: Not count first offset
            noffset = paint_beg - a->drawnums[0];
        }
        else {
            EndPaint(hWnd, &ps);
            return;
        }
    }
    else {
        if (a->drawnums[0] != paint_beg) {
            a = a->prev;
            noffset = paint_beg - a->drawnums[0];
        }
    }

    SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));

    for (i = paint_beg; i <= paint_end; i++) {
        x = Globals.CharW * (0 - horiz_pos);
        y = Globals.CharH * (i - vert_pos);

        printf("x = %i, y = %i\n", x, y);

        drawoffset = noffset * maxlen;
        if (Globals.isWrapLongLines) {
            drawremain = a->str.len - drawoffset;
            drawlen = drawremain >= maxlen ? maxlen : drawremain;
            /*
            drawlen = (a->str.len - noffset * maxlen) >= maxlen ?
                maxlen : (a->str.len - noffset * maxlen) % maxlen;
            */
        }
        else {
            drawlen = a->str.len;
        }
        TextOut(hDC, x, y, a->str.data + drawoffset, drawlen);
        noffset++;
        if (noffset >= a->noffsets) {
            if (a == Globals.TextList.last)
                break;
            noffset = 0;
            a = a->next;
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
    SCROLLINFO scroll_info;
    int vert_pos;

    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask = SIF_ALL;
    GetScrollInfo(hWnd, SB_VERT, &scroll_info);

    vert_pos = scroll_info.nPos;

    switch (Code) {
        case SB_TOP:
            scroll_info.nPos = scroll_info.nMin;
            break ;
        case SB_BOTTOM:
            scroll_info.nPos = scroll_info.nMax;
            break ;
        case SB_LINEUP:
            scroll_info.nPos--;
            break;
        case SB_LINEDOWN:
            scroll_info.nPos++;
            break;
        case SB_PAGEUP:
            scroll_info.nPos -= scroll_info.nPage;
            break;
        case SB_PAGEDOWN:
            scroll_info.nPos += scroll_info.nPage;
            break;
        case SB_THUMBTRACK:
            scroll_info.nPos = scroll_info.nTrackPos;
            break;
    }

    scroll_info.fMask = SIF_POS;
    SetScrollInfo(hWnd, SB_VERT, &scroll_info, true);

    GetScrollInfo(hWnd, SB_VERT, &scroll_info);
    if (scroll_info.nPos != vert_pos) {
        ScrollWindow(hWnd, 0,
                     Globals.CharH * (vert_pos - scroll_info.nPos),
                     NULL, NULL);

        UpdateWindow(hWnd);
    }
}

/***********************************************************************
 *
 *           NOTEPAD_OnHScroll
 */
static VOID NOTEPAD_OnHScroll(HWND hWnd, HWND hWndCtl, uint Code, int Pos )
{
    SCROLLINFO scroll_info;
    int horiz_pos;

    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask  = SIF_ALL;
    GetScrollInfo (hWnd, SB_HORZ, &scroll_info);

    horiz_pos = scroll_info.nPos;

    switch (Code) {
        case SB_LINELEFT:
            scroll_info.nPos--;
            break ;
        case SB_LINERIGHT:
            scroll_info.nPos++;
            break ;
        case SB_PAGELEFT:
            scroll_info.nPos -= scroll_info.nPage;
            break ;
        case SB_PAGERIGHT:
            scroll_info.nPos += scroll_info.nPage;
            break ;
        case SB_THUMBPOSITION:
            scroll_info.nPos = scroll_info.nTrackPos;
            break ;
    }

    scroll_info.fMask = SIF_POS;
    SetScrollInfo(hWnd, SB_HORZ, &scroll_info, true);

    GetScrollInfo (hWnd, SB_HORZ, &scroll_info);
    if (scroll_info.nPos != horiz_pos) {
        ScrollWindow(hWnd,
                     Globals.CharW * (horiz_pos - scroll_info.nPos), 0,
                     NULL, NULL);
        UpdateWindow(hWnd);
    }
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
/*case WM_QUERYENDSESSION:
            if (DoCloseFile()) {
                return 1;
            }
            break;*/
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
    Globals.hInstance = hInstance;
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
    if (main_rect.left   >= info.rcWork.right ||
        main_rect.top    >= info.rcWork.bottom ||
        main_rect.right  <  info.rcWork.left ||
        main_rect.bottom <  info.rcWork.top)
            x = y = CW_USEDEFAULT;

    Globals.hMainWnd = CreateWindow(className, winName,
                                    WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
                                    x, y,
                                    main_rect.right - main_rect.left,
                                    main_rect.bottom - main_rect.top,
                                    NULL, NULL, Globals.hInstance, NULL);
    if (!Globals.hMainWnd) {
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
