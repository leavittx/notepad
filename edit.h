/* Lev Panov, 2057/2, October 2010 */

#ifndef EDIT_H
#define EDIT_H

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    char *data; // String data
    int len;    // String lengh
} String;       // String

typedef struct tagTextItem {
    String str;    // String and it's lengh
    int *drawnums; // Ordernal number of the wrapped line into the entire set of wrapped lines
    int *offsets;  // Offsets to wrap long lines
    int noffsets;  // Number of offsets
    struct tagTextItem *prev, *next; // Previous and next elements
} TextItem; // This is a single string from file with some additionl info

typedef struct {
  TextItem *first, *last;  // First and last strings in text
  int nDrawLines;          // Number of lines to draw
  int LongestStringLength; // Length of the longest line in text, this is needed for horizontal scrolling
} Text; // Text (which was in file)

typedef enum {
    DIR_UP,        // Move the cursor up
    DIR_DOWN,      // Move the cursor down
    DIR_RIGHT,     // Move the cursor right
    DIR_LEFT,      // Move the cursor left
    DIR_HOME,      // Go to the beginning of the current line
    DIR_END,       // Go to the end of current line
    DIR_NEXT,      // Go to the next page of document
    DIR_PRIOR,     // Go to the previous page of document
    DIR_TEXT_HOME, // Go to the beginning of document
    DIR_TEXT_END   // Go to the end of document
} DIR; // Cursor directions

void EDIT_AddTextItem(FILE *f, int len); // Read a string from file
void EDIT_CountOffsets(void);            // Count offsets, drawnums, etc
void EDIT_ClearTextList(void);           // Remove text from memory

void EDIT_MoveCaret(DIR dir); // Move caret
void EDIT_FixCaret(void);     // Count draw-related caret line number and position based on the absolute values

void EDIT_DoBackspace(void);       // Backspace
void EDIT_DoReturn(void);          // Enter
void EDIT_InsertCharacter(char c); // Instert printable character

#endif // EDIT_H
