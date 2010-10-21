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
            // TODO -- show some error message
            ExitProcess(1);
        }
        (*first)->prev = (*first)->next = NULL;
        cur = *last = *first;
    }
    else {
        cur = *last;
        if ((cur->next = malloc(sizeof(TextItem))) == NULL) {
            // TODO -- show some error message
            ExitProcess(1);
        }
        cur = cur->next;
        cur->prev = *last;
        cur->next = NULL;
        *last = cur;
    }

    if ((cur->str.data = malloc(len + 1)) == NULL) {
        // TODO -- show some error message
        ExitProcess(1);
    }

    fread(cur->str.data, 1, len, f);
    cur->str.data[len] = '\0';
    cur->str.len = len;
}

void EDIT_CountOffsets(void)
{
    Globals.TextList.nDrawLines = 0;

    if (Globals.TextList.first == NULL)
        return;

    // Wrap mode disabled
    if (!Globals.isWrapLongLines) {
        int i = 0;
        for (TextItem *a = Globals.TextList.first; ; a = a->next, i++) {
            if (a->drawnums != NULL)
                free(a->drawnums);
            if (a->offsets != NULL)
                free(a->offsets);

            a->drawnums = malloc(sizeof(int));
            a->offsets = malloc(sizeof(int));

            a->drawnums[0] = i;
            a->offsets[0] = 0;
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

        if (a->drawnums != NULL)
            free(a->drawnums);
        if (a->offsets != NULL)
            free(a->offsets);

        a->drawnums = malloc(noffsets * sizeof(int));
        a->offsets = malloc(noffsets * sizeof(int));

        for (int i = 0; i < noffsets; i++) {
            a->drawnums[i] = drawnum++;
            a->offsets[i] = i * maxlen;
        }
        a->noffsets = noffsets;
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

void EDIT_ClearTextList(void)
{
    if (Globals.TextList.first == NULL)
        return;

    for (TextItem *a = Globals.TextList.first, *b; ; a = b) {
        if (a != Globals.TextList.last)
            b = a->next;
        else
            b = NULL;
        // Free all allocated memory
        free(a->str.data);
        free(a->drawnums);
        free(a->offsets);
        free(a);
        // Break on the last element
        if (b == NULL)
            break;
    }
    // The list is clear now
    Globals.TextList.first = NULL;
}
