/* Lev Panov, 2057/2, October 2010 */

#include <windows.h>
#include <stdio.h>

#include "main.h"
#include "edit.h"

bool AddTextItem(FILE *f, int len)
{
    TextItem **first = &Globals.TextList.first,
             **last = &Globals.TextList.last,
              *cur;

    if (*first == NULL) {
        if ((*first = malloc(sizeof(TextItem))) == NULL) {
            return false;
        }
        (*first)->prev = (*first)->next = NULL;
        cur = *last = *first;
    }
    else {
        cur = *last;
        if ((cur->next = malloc(sizeof(TextItem))) == NULL) {
            return false;
        }
        cur = cur->next;
        cur->prev = *last;
        cur->next = NULL;
        *last = cur;
    }

    if ((cur->str.data = malloc(len + 1)) == NULL) {
        return false;
    }

    fread(cur->str.data, 1, len, f);
    cur->str.data[len] = '\0';
    cur->str.len = len;

    // Debug
    //fputs(cur->str.data, stdout);

    return true;
}
