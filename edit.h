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
    struct tagTextItem *prev, *next;
} TextItem;

typedef struct {
  TextItem *first, *last;
} Text;

bool AddTextItem(FILE *f, int len);

#endif // EDIT_H
