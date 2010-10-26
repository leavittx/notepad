/* Lev Panov, 2057/2, October 2010 */

#ifndef EDIT_H
#define EDIT_H

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    char *data;
    int len;
} String;

typedef struct tagTextItem {
    String str;
    int *drawnums;
    int *offsets;
    int noffsets;
    struct tagTextItem *prev, *next;
} TextItem;

typedef struct {
  TextItem *first, *last;
  int nDrawLines;
  int LongestStringLength;
} Text;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_RIGHT,
    DIR_LEFT
} DIR;

void EDIT_AddTextItem(FILE *f, int len);
void EDIT_CountOffsets(void);
void EDIT_ClearTextList(void);

void EDIT_MoveCaret(DIR dir);
void EDIT_FixCaret(void);

#endif // EDIT_H
