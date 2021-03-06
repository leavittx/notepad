/* Lev Panov, 2057/2, October 2010 */

#define WINVER 0x0500

#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>
//#include <time.h>
#include <ctype.h> // For isprint()

#include "main.h"
#include "dialog.h"
#include "notepad_res.h"

NOTEPAD_GLOBALS Globals; // Notepad globals
static RECT main_rect;   // Main window rect

/***********************************************************************
 *          SetFileName
 *
 *  Sets global file name and title
 *
 *  ARGUMENTS:
 *    - file name:
 *         const char *FileName
 *  RETURNS: none
 */
void SetFileName(const char *FileName)
{
    lstrcpy(Globals.FileName, FileName);
    Globals.FileTitle[0] = 0;
    GetFileTitle(FileName, Globals.FileTitle, sizeof(Globals.FileTitle));
}

/***********************************************************************
 *           NOTEPAD_SetParams
 *
 *  Sets some notepad params
 *
 *  ARGUMENTS: none
 *  RETURNS: none
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

    Globals.isModified = false;
    Globals.isWrapLongLines = true;
    Globals.EOL_type = EOL_LF;
}

/***********************************************************************
 *          NOTEPAD_OnCreate
 *
 *  WM_CREATE window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - structure with creation data (not used):
 *         CREATESTRUCT *cs
 *  RETURNS:
 *      (bool) true to continue creation window, false to terminate
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

    ReleaseDC(hWnd, hDC);

/*
    // Make some control elements darker (not works on win)
    int aElements[] = { COLOR_MENU, COLOR_MENUTEXT,
                        COLOR_SCROLLBAR, COLOR_WINDOW };
    DWORD aNewColors[ARRAY_SIZE(aElements)];
    aNewColors[0] = RGB(0, 0, 0);
    aNewColors[1] = RGB(128, 128, 128);
    aNewColors[2] = RGB(0, 0, 0);
    aNewColors[3] = RGB(254, 254, 254);
    SetSysColors(ARRAY_SIZE(aElements), aElements, aNewColors);
*/
/*
    // LSD
    int aElements[31];
    DWORD aNewColors[ARRAY_SIZE(aElements)];
    while (1) {
        srand(time(0));
        for (int i = 0; i < ARRAY_SIZE(aElements); i++) {
            aElements[i] = i;
            aNewColors[i] = RGB(rand()%255, rand()%255, rand()%255);
        }
        SetSysColors(ARRAY_SIZE(aElements), aElements, aNewColors);
        Sleep(50);
    }
    ExitProcess(0);
*/

    return true;
}

/***********************************************************************
 *          NOTEPAD_SetFocus
 *
 *  WM_SETFOCUS window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - (not used):
 *         HWND lostFocusWnd
 *  RETURNS: none
 */
static void NOTEPAD_OnSetFocus(HWND hWnd, HWND lostFocusWnd)
{
    CreateCaret(hWnd, NULL, 0, Globals.CharH);
    //SetCaretPos(marginX + caretXpos * tm.tmAveCharWidth, caretYpos * tm.tmHeight + marginY);
    SendMessage(Globals.hMainWnd, WM_KEYDOWN, 0, 0); // Restore caret position
    ShowCaret(hWnd);
}

/***********************************************************************
 *          NOTEPAD_OnKillFocus
 *
 *  WM_KILLFOCUS window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - (not used):
 *         HWND recvFocusWnd
 *  RETURNS: none
 */
static void NOTEPAD_OnKillFocus(HWND hWnd, HWND recvFocusWnd)
{
    HideCaret(hWnd);
    DestroyCaret();
}

/***********************************************************************
 *          UpdateStuff
 *
 *  Update scroll info (if needed) and set caret position
 *
 *  ARGUMENTS:
 *    - do we need to update scroll info
 *         bool isFixScroll
 *  RETURNS: none
 */
static void UpdateStuff(bool isFixScroll)
{
    SCROLLINFO vert_scroll_info, horz_scroll_info;

    if (isFixScroll) {
        // Get vertical scroll info
        vert_scroll_info.cbSize = sizeof(vert_scroll_info);
        vert_scroll_info.fMask  = SIF_ALL;
        GetScrollInfo(Globals.hMainWnd, SB_VERT, &vert_scroll_info);
        // Get horizontal scroll info
        horz_scroll_info.cbSize = sizeof(horz_scroll_info);
        horz_scroll_info.fMask  = SIF_ALL;
        GetScrollInfo(Globals.hMainWnd, SB_HORZ, &horz_scroll_info);

        //SendMessage(hWnd, WM_VSCROLL, SB_THUMBTRACK, MAKELONG(0, MAKEWORD(0, 222)));

        // Fix vertical scroll
        if (Globals.CaretCurLine == vert_scroll_info.nPos + vert_scroll_info.nPage) {
            SendMessage(Globals.hMainWnd, WM_VSCROLL, SB_LINEDOWN, 0);
        }
        else if (Globals.CaretCurLine > vert_scroll_info.nPos + vert_scroll_info.nPage) {
            int vert_pos = vert_scroll_info.nPos;

            vert_scroll_info.nPos = Globals.CaretCurLine - vert_scroll_info.nPage + 1;

            vert_scroll_info.fMask = SIF_POS;
            SetScrollInfo(Globals.hMainWnd, SB_VERT, &vert_scroll_info, true);

            ScrollWindow(Globals.hMainWnd,
                         0, Globals.CharH * (vert_pos - vert_scroll_info.nPos),
                         NULL, NULL);
            UpdateWindow(Globals.hMainWnd);
        }
        else if (Globals.CaretCurLine == vert_scroll_info.nPos - 1) {
            SendMessage(Globals.hMainWnd, WM_VSCROLL, SB_LINEUP, 0);
        }
        else if (Globals.CaretCurLine < vert_scroll_info.nPos) {
            int vert_pos = vert_scroll_info.nPos;

            vert_scroll_info.nPos = Globals.CaretCurLine;

            vert_scroll_info.fMask = SIF_POS;
            SetScrollInfo(Globals.hMainWnd, SB_VERT, &vert_scroll_info, true);

            ScrollWindow(Globals.hMainWnd,
                         0, Globals.CharH * (vert_pos - vert_scroll_info.nPos),
                         NULL, NULL);
            UpdateWindow(Globals.hMainWnd);
        }

        // Fix horizontal scroll
        if (!Globals.isWrapLongLines) {
            if (Globals.CaretCurPos == horz_scroll_info.nPos + horz_scroll_info.nPage) {
                SendMessage(Globals.hMainWnd, WM_HSCROLL, SB_LINERIGHT, 0);
            }
            else if (Globals.CaretCurPos > horz_scroll_info.nPos + horz_scroll_info.nPage) {
                int horz_pos = horz_scroll_info.nPos;

                horz_scroll_info.nPos = Globals.CaretCurPos - horz_scroll_info.nPage + 1;

                horz_scroll_info.fMask = SIF_POS;
                SetScrollInfo(Globals.hMainWnd, SB_HORZ, &horz_scroll_info, true);

                ScrollWindow(Globals.hMainWnd,
                             Globals.CharW * (horz_pos - horz_scroll_info.nPos), 0,
                             NULL, NULL);
                UpdateWindow(Globals.hMainWnd);
            }
            else if (Globals.CaretCurPos == horz_scroll_info.nPos - 1) {
                SendMessage(Globals.hMainWnd, WM_HSCROLL, SB_LINELEFT, 0);
            }
            else if (Globals.CaretCurPos < horz_scroll_info.nPos) {
                int horz_pos = horz_scroll_info.nPos;

                horz_scroll_info.nPos = Globals.CaretCurPos;

                horz_scroll_info.fMask = SIF_POS;
                SetScrollInfo(Globals.hMainWnd, SB_HORZ, &horz_scroll_info, true);

                ScrollWindow(Globals.hMainWnd,
                             Globals.CharW * (horz_pos - horz_scroll_info.nPos), 0,
                             NULL, NULL);
                UpdateWindow(Globals.hMainWnd);
            }
        }
    }

    // Get vertical scroll info
    vert_scroll_info.cbSize = sizeof(vert_scroll_info);
    vert_scroll_info.fMask  = SIF_ALL;
    GetScrollInfo(Globals.hMainWnd, SB_VERT, &vert_scroll_info);
    // Get horizontal scroll info
    horz_scroll_info.cbSize = sizeof(horz_scroll_info);
    horz_scroll_info.fMask  = SIF_ALL;
    GetScrollInfo(Globals.hMainWnd, SB_HORZ, &horz_scroll_info);

    SetCaretPos((Globals.CaretCurPos - horz_scroll_info.nPos) * Globals.CharW,
                (Globals.CaretCurLine - vert_scroll_info.nPos) * Globals.CharH);
}

/***********************************************************************
 *          NOTEPAD_OnSize
 *
 *  WM_SIZE window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - (not used):
 *         uint State
 *    - new width:
 *         int W
 *    - new height:
 *         int H
 *  RETURNS: none
 */
static void NOTEPAD_OnSize(HWND hWnd, uint State, int W, int H)
{
    SCROLLINFO scroll_info;

    Globals.W = W;
    Globals.H = H;

    if (W == 0 || H == 0)
        return;

    EDIT_CountOffsets();

    // Vertical scroll
    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask = SIF_PAGE | SIF_RANGE;
    scroll_info.nMin = 0;
    scroll_info.nMax = Globals.TextList.nDrawLines - 1;
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
    scroll_info.nPage = W / Globals.CharW + 1;
    SetScrollInfo(hWnd, SB_HORZ, &scroll_info, true);

    // Fix caret position
    EDIT_FixCaret();
    UpdateStuff(false);
    //SendMessage(Globals.hMainWnd, WM_KEYDOWN, 0, 0);

    InvalidateRect(hWnd, NULL, false);
}

/***********************************************************************
 *          NOTEPAD_OnPaint
 *
 *  WM_PAINT window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *  RETURNS: none
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

    GetClientRect(hWnd, &rc);
    hDC = BeginPaint(hWnd, &ps);
    FillRect(hDC, &rc, GetStockObject(BLACK_BRUSH));

    // TODO -- clear window??
    if (Globals.FileName[0] == '\0' && Globals.TextList.first == NULL) {
        EndPaint(hWnd, &ps);
        return;
    }

    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask  = SIF_POS;
    GetScrollInfo(hWnd, SB_VERT, &scroll_info);
    vert_pos = scroll_info.nPos;
    GetScrollInfo(hWnd, SB_HORZ, &scroll_info);
    horiz_pos = scroll_info.nPos;

    paint_beg = max(0, vert_pos + ps.rcPaint.top / Globals.CharH);
    paint_end = min(Globals.TextList.nDrawLines - 1,
                    vert_pos + ps.rcPaint.bottom / Globals.CharH);

#ifdef DEBUG
    printf("paint: %i %i\n", paint_beg, paint_end);
#endif

    TextItem *a;
    for (a = Globals.TextList.first;
         a->drawnums[0] < paint_beg && a != Globals.TextList.last;
         a = a->next);

    if (a == Globals.TextList.last) {
        if (a->drawnums[0] + a->noffsets - 1 >= paint_beg) // -1: Not count first offset
            noffset = paint_beg - a->drawnums[0];
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
    SetTextColor(hDC, RGB(0, 255, 0));
    //SetTextColor(hDC, RGB(128, 128, 128));
    //SetTextColor(hDC, RGB(0xff, 0x84, 0x0)); // Orange
    SetBkColor(hDC, RGB(0, 0, 0));

    for (int i = paint_beg; i <= paint_end; i++) {
        x = Globals.CharW * (0 - horiz_pos);
        y = Globals.CharH * (i - vert_pos);

        drawoffset = noffset * maxlen;
        if (Globals.isWrapLongLines) {
            drawremain = a->str.len - drawoffset;
            drawlen = drawremain >= maxlen ? maxlen : drawremain;
            /*
            drawlen = (a->str.len - noffset * maxlen) >= maxlen ?
                maxlen : (a->str.len - noffset * maxlen) % maxlen;
            */
        }
        else
            drawlen = a->str.len;

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
 *          NOTEPAD_OnVScroll
 *
 *  WM_VSCROLL window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - (not used):
 *         HWND hWndCtl
 *    - scroll code:
 *         uint Code
 *    - (not used):
 *         int Pos
 *  RETURNS: none
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
            break;

        case SB_BOTTOM:
            scroll_info.nPos = scroll_info.nMax;
            break;

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

        case SB_THUMBPOSITION:
            scroll_info.nPos = scroll_info.nTrackPos;
            break;
    }

    scroll_info.fMask = SIF_POS;
    SetScrollInfo(hWnd, SB_VERT, &scroll_info, true);

    GetScrollInfo(hWnd, SB_VERT, &scroll_info);
    if (scroll_info.nPos != vert_pos) {
        ScrollWindow(hWnd,
                     0, Globals.CharH * (vert_pos - scroll_info.nPos),
                     NULL, NULL);
        UpdateWindow(hWnd);

        UpdateStuff(false);
    }
}

/***********************************************************************
 *          NOTEPAD_OnHScroll
 *
 *  WM_HSCROLL window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - (not used):
 *         HWND hWndCtl
 *    - scroll code:
 *         uint Code
 *    - (not used):
 *         int Pos
 *  RETURNS: none
 */
static VOID NOTEPAD_OnHScroll(HWND hWnd, HWND hWndCtl, uint Code, int Pos)
{
    SCROLLINFO scroll_info;
    int horz_pos;

    scroll_info.cbSize = sizeof(scroll_info);
    scroll_info.fMask  = SIF_ALL;
    GetScrollInfo (hWnd, SB_HORZ, &scroll_info);

    horz_pos = scroll_info.nPos;

    switch (Code) {
        case SB_LINELEFT:
            scroll_info.nPos--;
            break;

        case SB_LINERIGHT:
            scroll_info.nPos++;
            break;

        case SB_PAGELEFT:
            scroll_info.nPos -= scroll_info.nPage;
            break;

        case SB_PAGERIGHT:
            scroll_info.nPos += scroll_info.nPage;
            break;

        case SB_THUMBTRACK:
            scroll_info.nPos = scroll_info.nTrackPos;
            break;

        case SB_THUMBPOSITION:
            scroll_info.nPos = scroll_info.nTrackPos;
            break;
    }

    scroll_info.fMask = SIF_POS;
    SetScrollInfo(hWnd, SB_HORZ, &scroll_info, true);

    GetScrollInfo (hWnd, SB_HORZ, &scroll_info);
    if (scroll_info.nPos != horz_pos) {
        ScrollWindow(hWnd,
                     Globals.CharW * (horz_pos - scroll_info.nPos), 0,
                     NULL, NULL);
        UpdateWindow(hWnd);

        UpdateStuff(false);
    }
}

/***********************************************************************
 *          NOTEPAD_OnKeyDown
 *
 *  WM_KEYDOWN window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - virtual key code:
 *         uint VKey
 *    - (not used):
 *         bool Down
 *    - (not used):
 *         int Repeat
 *    - (not used):
 *         uint flags
 *  RETURNS: none
 */
static void NOTEPAD_OnKeyDown(HWND hWnd, uint VKey, bool Down, int Repeat, uint flags)
{
    switch (VKey) {
        case VK_UP:
            EDIT_MoveCaret(DIR_UP);
            break;

        case VK_DOWN:
            EDIT_MoveCaret(DIR_DOWN);
            break;

        case VK_RIGHT:
            EDIT_MoveCaret(DIR_RIGHT);
            break;

        case VK_LEFT:
            EDIT_MoveCaret(DIR_LEFT);
            break;

        case VK_NEXT: // Page Down
            EDIT_MoveCaret(DIR_NEXT);
            break;

        case VK_PRIOR: // Page Up
            EDIT_MoveCaret(DIR_PRIOR);
            break;

        case VK_HOME:
            EDIT_MoveCaret(DIR_HOME);
            break;

        case VK_END:
            EDIT_MoveCaret(DIR_END);
            break;

        case VK_ESCAPE:
            //SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        case VK_DELETE:
            if (Globals.CaretCurLine == Globals.TextList.nDrawLines - 1 &&
                Globals.CaretAbsPos == Globals.TextList.last->str.len)
                    return;
            EDIT_MoveCaret(DIR_RIGHT);
            EDIT_DoBackspace();
            EDIT_MoveCaret(DIR_LEFT);
            SendMessage(hWnd, WM_SIZE, 0, MAKELONG(Globals.W, Globals.H));
            //InvalidateRect(hWnd, NULL, false);
            break;

        case VK_RETURN: // Enter hit
            EDIT_DoReturn();
            SendMessage(hWnd, WM_SIZE, 0, MAKELONG(Globals.W, Globals.H));
            //InvalidateRect(hWnd, NULL, false);
            Globals.isModified = true;
            break;

        default:
            return;
    }
    UpdateStuff(true);
}

/***********************************************************************
 *          NOTEPAD_OnChar
 *
 *  WM_CHAR window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - input character:
 *         unsigned char Ch
 *    - (not used):
 *         int cRepeat
 *  RETURNS: none
 */
static void NOTEPAD_OnChar(HWND hWnd, unsigned char Ch, int cRepeat)
{
      switch (Ch)
      {
          case '\b': // Backspace
            EDIT_DoBackspace();
            SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, 0);
            SendMessage(hWnd, WM_SIZE, 0, MAKELONG(Globals.W, Globals.H));
            //InvalidateRect(hWnd, NULL, false);
            break;

          default:
            if (isprint(Ch) || (Ch >= 192 && Ch <= 255)) {
                EDIT_InsertCharacter(Ch);
                SendMessage(hWnd, WM_KEYDOWN, VK_RIGHT, 0);
                SendMessage(hWnd, WM_SIZE, 0, MAKELONG(Globals.W, Globals.H));
                //InvalidateRect(hWnd, NULL, false);
                Globals.isModified = true;
            }
            break;
      }
}

/***********************************************************************
 *          NOTEPAD_OnMenuCommand
 *
 *  All handling of main menu events
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - menu command id:
 *         int Id
 *    - (not used):
 *         HWND hwndCtl
 *    - (not used):
 *         uint codeNotify
 *  RETURNS: none
 */
static void NOTEPAD_OnMenuCommand(HWND hwnd, int Id, HWND hwndCtl, uint codeNotify)
{
    switch (Id) {
        case CMD_NEW:     DIALOG_FileNew();    break;
        case CMD_OPEN:    DIALOG_FileOpen();   break;
        case CMD_SAVE:    DIALOG_FileSave();   break;
        case CMD_SAVE_AS: DIALOG_FileSaveAs(); break;
        case CMD_EXIT:    DIALOG_FileExit();   break;
        case CMD_WRAP:    DIALOG_EditWrap();   break;

        case CMD_TEXT_HOME:
            EDIT_MoveCaret(DIR_TEXT_HOME);
            UpdateStuff(true);
            break;

        case CMD_TEXT_END:
            EDIT_MoveCaret(DIR_TEXT_END);
            UpdateStuff(true);
            break;

        default:          break;
    }
}

/***********************************************************************
 *          NOTEPAD_OnDropFiles
 *
 *  WM_DROPFILES window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *    - drop info:
 *         HDROP hDrop
 *  RETURNS: none
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
 *          NOTEPAD_OnClose
 *
 *  WM_CLOSE window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *  RETURNS: none
 */
static void NOTEPAD_OnClose(HWND hWnd)
{
    if (DoCloseFile())
        DestroyWindow(hWnd);
}

/***********************************************************************
 *          NOTEPAD_OnDestroy
 *
 *  WM_DESTROY window message handle function
 *
 *  ARGUMENTS:
 *    - handle of window:
 *         HWND hWnd
 *  RETURNS: none
 */
static void NOTEPAD_OnDestroy(HWND hWnd)
{
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
 *           NOTEPAD_WndProc
 */
static LRESULT WINAPI NOTEPAD_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        HANDLE_MSG(hWnd, WM_CREATE, NOTEPAD_OnCreate);
        HANDLE_MSG(hWnd, WM_SETFOCUS, NOTEPAD_OnSetFocus);
        HANDLE_MSG(hWnd, WM_KILLFOCUS, NOTEPAD_OnKillFocus);
        HANDLE_MSG(hWnd, WM_SIZE, NOTEPAD_OnSize);
        HANDLE_MSG(hWnd, WM_PAINT, NOTEPAD_OnPaint);
        HANDLE_MSG(hWnd, WM_VSCROLL, NOTEPAD_OnVScroll);
        HANDLE_MSG(hWnd, WM_HSCROLL, NOTEPAD_OnHScroll);
        HANDLE_MSG(hWnd, WM_KEYDOWN, NOTEPAD_OnKeyDown);
        HANDLE_MSG(hWnd, WM_CHAR, NOTEPAD_OnChar);
        HANDLE_MSG(hWnd, WM_COMMAND, NOTEPAD_OnMenuCommand);
        HANDLE_MSG(hWnd, WM_DROPFILES, NOTEPAD_OnDropFiles);
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

/***********************************************************************
 *          AlertFileDoesNotExist
 *
 *  Alert that file does not exist
 *
 *  ARGUMENTS:
 *    - name of file:
 *         const char *FileName
 *  RETURNS:
 *      (int) user choice
 */
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

   return Result;
}

/***********************************************************************
 *          HandleCommandLine
 *
 *  Handle command line options
 *
 *  ARGUMENTS:
 *    - command line options passed to the program (as string):
 *         char *cmdline
 *  RETURNS: none
 */
static void HandleCommandLine(char *cmdline)
{
    /* file name is passed in the command line */
    if (*cmdline) {
        /* Remove double-quotes from filename */
        /* Double-quotes are not allowed in Windows filenames */
        if (cmdline[0] == '"')
        {
            char *wc;
            cmdline++;
            wc = cmdline;
            while (*wc && *wc != '"') wc++;
            *wc = 0;
        }
        if (FileExists(cmdline)) {
            DoOpenFile(cmdline);
            InvalidateRect(Globals.hMainWnd, NULL, false);
        }
        else {
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
}

/***********************************************************************
 *           WinMain
 */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE prev, char *cmdline, int show)
{
    MSG msg;
    HACCEL accel;
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
    wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)BLACK_BRUSH;//(HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE(MAIN_MENU);
    wc.lpszClassName = className;

    if (!RegisterClassEx(&wc))
        return false;

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
    //ShowWindow(Globals.hMainWnd, SW_HIDE);
    ShowWindow(Globals.hMainWnd, SW_SHOW);
    UpdateWindow(Globals.hMainWnd);
    DragAcceptFiles(Globals.hMainWnd, true);
    HandleCommandLine(cmdline);

    accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(ID_ACCEL));

    while (GetMessage(&msg, 0, 0, 0)) {
        if (!TranslateAccelerator(Globals.hMainWnd, accel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}
