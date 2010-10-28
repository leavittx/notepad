/* Lev Panov, 2057/2, October 2010 */

#ifndef MAIN_H
#define MAIN_H

#include "edit.h"
#include "notepad_res.h"

typedef unsigned int uint; // Define short name for unsigned int type

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) // Array size macro

#define MAX_STRING_LEN 255 // Maximum string lengh

#define LF '\n' // LF character
#define CR '\r' // CR character

typedef enum {
    EOL_LF,  // Unix style line break
    EOL_CRLF // Windows style line break
} EOL_TYPE;  // Type of line break

typedef struct {
    HANDLE hInstance;                 // Program  instance
    HWND   hMainWnd;                  // Program main window
    int    W, H;                      // Current window width and height
    bool   isWrapLongLines;           // Do we need to wrap long lines now
    bool   isModified;                // Was the buffer modified or not
    char   FileName[MAX_PATH];        // Path of current file
    char   FileTitle[MAX_PATH];       // Title of current file
    char   Filter[MAX_STRING_LEN];    // Open and save dialog file filter
    EOL_TYPE EOL_type;                // Type of line break in current file
    Text   TextList;                  // List that contains lines from file and some info
    int    CharW, CharH;              // Character width and height
    int    CaretAbsLine, CaretAbsPos; // Caret absolute number of line and position in line
    int    CaretCurLine, CaretCurPos; // Caret draw-related number of line and position
} NOTEPAD_GLOBALS; // Notepad globals

extern NOTEPAD_GLOBALS Globals; // Extern notepad globals

void SetFileName(const char *FileName); // Set file name (path) and title

#endif // MAIN_H
