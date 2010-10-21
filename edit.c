/* Lev Panov, 2057/2, October 2010 */

#include <windows.h>
#include <stdio.h>

#include "main.h"
#include "edit.h"

void EDIT_AddTextItem(FILE *f, int len)
{
    TextItem **first = &Globals.TextList.first,
             **last = &Globals.TextList.last,
              *cur;

    if (*first == NULL) {
        if ((*first = malloc(sizeof(TextItem))) == NULL) {
            // Show some error message
            ExitProcess(1);
        }
        (*first)->prev = (*first)->next = NULL;
        cur = *last = *first;
    }
    else {
        cur = *last;
        if ((cur->next = malloc(sizeof(TextItem))) == NULL) {
            // Show some error message
            ExitProcess(1);
        }
        cur = cur->next;
        cur->prev = *last;
        cur->next = NULL;
        *last = cur;
    }

    if ((cur->str.data = malloc(len + 1)) == NULL) {
        // Show some error message
        ExitProcess(1);
    }

    fread(cur->str.data, 1, len, f);
    cur->str.data[len] = '\0';
    cur->str.len = len;
}

void EDIT_CountOffsets(void)
{
    Globals.TextList.nDrawLines = 0;

    // Wrap mode disabled
    if (!Globals.isWrapLongLines) {
        int i = 0;
        for (TextItem *a = Globals.TextList.first; ; a = a->next, i++) {
            if (a->noffsets >= 1) {
                //free(a->offsets);
                //malloc()
            }
            else {
                a->drawnums = malloc(sizeof(int));
                a->offsets = malloc(sizeof(int));
                a->offsets[0] = 0;
            }
            a->drawnums[0] = i;
            a->noffsets = 1;
            Globals.TextList.nDrawLines++;

            if (a == Globals.TextList.last)
                break;
        }
        return;
    }

    // Wrap mode enabled
    int maxlen = Globals.W / Globals.CharW;
    int drawnum = 0;
    for (TextItem *a = Globals.TextList.first; ; a = a->next) {
        int noffsets = a->str.len / maxlen + 1;
        if (noffsets > a->noffsets) {
            if (a->drawnums != NULL)
                free(a->drawnums);
            a->drawnums = malloc(noffsets * sizeof(int));
            if (a->offsets != NULL)
                free(a->offsets);
            a->offsets = malloc(noffsets * sizeof(int));
        }
        a->noffsets = noffsets;
        for (int i = 0; i < noffsets; i++) {
            a->drawnums[i] = drawnum++;
            a->offsets[i] = i * maxlen;
        }
        Globals.TextList.nDrawLines += noffsets;

        if (a == Globals.TextList.last)
            break;
    }

#ifdef DEBUG
    int i = 0;
    for (TextItem *a = Globals.TextList.first; ; a = a->next) {
        printf("line %i: %i offsets -- ", i++, a->noffsets);
        for (int i = 0; i < a->noffsets; i++)
            printf("%i[%i] ", a->offsets[i], a->drawnums[i]);
        printf("\n");
        if (a == Globals.TextList.last)
            break;
    }
#endif
}
