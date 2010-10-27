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
            //if (a->drawnums != NULL)
                free(a->drawnums);
            //if (a->offsets != NULL)
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

        //if (a->drawnums != NULL)
            free(a->drawnums);
        //if (a->offsets != NULL)
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

void EDIT_MoveCaret(DIR dir)
{
    TextItem *a, *prev;
    int noffset;
    int maxlen = Globals.W / Globals.CharW;
    int pageH = Globals.H / Globals.CharH + 1;
    int i, j;

    if (Globals.TextList.first == NULL)
        return;

    // Find current and previous string (where caret is)
    for (a = Globals.TextList.first;
         a->drawnums[0] < Globals.CaretCurLine; a = a->next) {
            if (a == Globals.TextList.last)
                break;
    }
    if (a == Globals.TextList.last) {
        if (a->drawnums[a->noffsets - 1] < Globals.CaretCurLine) // Should not happen
            return;
    }
    else {
        if (a->drawnums[0] != Globals.CaretCurLine)
            a = a->prev;
    }
    if (a != Globals.TextList.first) {
        prev = a->prev;
    }

    // Count current offset (for caret)
    if (Globals.isWrapLongLines)
        noffset = Globals.CaretAbsPos / maxlen;
    else
        noffset = 0;

    // Handle different caret move directions
    switch (dir)
    {
    case DIR_UP:
        if (Globals.CaretCurLine > 0)
        {
            Globals.CaretCurLine--;

            if (noffset == 0) {
                if (Globals.CaretAbsLine > 0) {
                    Globals.CaretAbsLine--;
                    if (Globals.CaretCurPos > prev->str.len - maxlen * (prev->noffsets - 1))
                        Globals.CaretCurPos = prev->str.len - maxlen * (prev->noffsets - 1);
                    Globals.CaretAbsPos = maxlen * (prev->noffsets - 1) + Globals.CaretCurPos;
                }
            }
            else {
                Globals.CaretAbsPos -= maxlen;
            }
        }
        break;

    case DIR_DOWN:
        if (Globals.CaretCurLine < Globals.TextList.nDrawLines - 1)
        {
            Globals.CaretCurLine++;

            if (noffset == a->noffsets - 1) {
                //if (Globals.CaretAbsLine > 0) {
                    Globals.CaretAbsLine++;
                    if (a->next->noffsets == 1)
                        if (Globals.CaretCurPos > a->next->str.len)
                            Globals.CaretCurPos = a->next->str.len;
                    Globals.CaretAbsPos = Globals.CaretCurPos;
                //}
            }
            else {
                if (Globals.CaretCurPos + maxlen <= a->str.len)
                    Globals.CaretAbsPos += maxlen;
                else {
                    Globals.CaretAbsPos = a->str.len;
                    Globals.CaretCurPos = a->str.len - maxlen * (a->noffsets - 1);
                }
            }
        }
        break;

    case DIR_RIGHT:
        if (noffset == a->noffsets - 1) {
            if (Globals.CaretAbsPos >= a->str.len) {
                if (Globals.CaretCurLine < Globals.TextList.nDrawLines - 1) {
                    Globals.CaretAbsLine++;
                    Globals.CaretCurLine++;
                    Globals.CaretAbsPos = 0;
                    Globals.CaretCurPos = 0;
                }
            }
            else {
                Globals.CaretAbsPos++;
                Globals.CaretCurPos++;
            }
        }
        else {
            if (Globals.CaretCurPos >= maxlen - 1) { // Behave like original notepad
                Globals.CaretCurLine++;
                Globals.CaretAbsPos++;
                Globals.CaretCurPos = 0;
            }
            else {
                Globals.CaretAbsPos++;
                Globals.CaretCurPos++;
            }
        }

        break;

    case DIR_LEFT:
        if (noffset == 0) {
            if (Globals.CaretCurPos <= 0) { // CaretCurPos = CaretAbsPos in this case
                if (Globals.CaretAbsLine > 0) {
                    Globals.CaretAbsLine--;
                    Globals.CaretCurLine--;
                    Globals.CaretAbsPos = prev->str.len;
                    Globals.CaretCurPos = prev->str.len - maxlen * (prev->noffsets - 1);
                }
            }
            else {
                Globals.CaretAbsPos--;
                Globals.CaretCurPos--;
            }
        }
        else {
            if (Globals.CaretCurPos <= 0) {
                Globals.CaretCurLine--;
                Globals.CaretAbsPos--;
                Globals.CaretCurPos = maxlen - 1;
            }
            else {
                Globals.CaretAbsPos--;
                Globals.CaretCurPos--;
            }
        }
        break;

    case DIR_HOME:
        Globals.CaretCurLine = a->drawnums[0];
        Globals.CaretAbsPos = 0;
        Globals.CaretCurPos = 0;
        break;

    case DIR_END:
        Globals.CaretCurLine = a->drawnums[a->noffsets - 1];
        Globals.CaretAbsPos = a->str.len;
        Globals.CaretCurPos = a->str.len - maxlen * (a->noffsets - 1);
        break;

    case DIR_NEXT:
        for (i = 0; i != pageH; ) {
            if (i == 0)
                for (j = noffset; i != pageH && j < a->noffsets; j++)
                    i++;
            else
                for (j = 0; i != pageH && j < a->noffsets; j++)
                    i++;
            if (a == Globals.TextList.last || i == pageH)
                break;
            a = a->next;
            Globals.CaretAbsLine++;
        }
        j--;
        Globals.CaretCurLine = a->drawnums[j];
        if (Globals.CaretCurPos > a->str.len - maxlen * j)
            Globals.CaretCurPos = a->str.len - maxlen * j;
        Globals.CaretAbsPos = Globals.CaretCurPos + maxlen * j;
        break;

    case DIR_PRIOR:
        for (i = 0; i != pageH; ) {
            if (i == 0)
                for (j = noffset; i != pageH && j >= 0; j--)
                    i++;
            else
                for (j = a->noffsets - 1; i != pageH && j >= 0; j--)
                    i++;
            if (a == Globals.TextList.first || i == pageH)
                break;
            a = a->prev;
            Globals.CaretAbsLine--;
        }
        j++;
        Globals.CaretCurLine = a->drawnums[j];
        if (Globals.CaretCurPos > a->str.len - maxlen * j)
            Globals.CaretCurPos = a->str.len - maxlen * j;
        Globals.CaretAbsPos = Globals.CaretCurPos + maxlen * j;
        break;
    }

#ifdef DEBUG
    printf("%5s curline: %i, curpos: %i, absline: %i, abspos: %i\n",
           dir == DIR_DOWN  ? "DOWN" :
           dir == DIR_LEFT  ? "LEFT" :
           dir == DIR_RIGHT ? "RIGHT": "UP",
           Globals.CaretCurLine,
           Globals.CaretCurPos,
           Globals.CaretAbsLine,
           Globals.CaretAbsPos);
#endif
}

void EDIT_FixCaret(void)
{
    TextItem *a;
    int noffset;
    int maxlen = Globals.W / Globals.CharW;
    int i = 0;

    if (Globals.TextList.first == NULL)
        return;

    // Find where caret is
    for (a = Globals.TextList.first;
         i < Globals.CaretAbsLine; a = a->next, i++) {
            if (a == Globals.TextList.last)
                break;
    }
    if (a == Globals.TextList.last) {
        if (i != Globals.CaretAbsLine) // Should not happen
            return;
    }

    // Count current offset (for caret)
    if (Globals.isWrapLongLines)
        noffset = Globals.CaretAbsPos / maxlen;
    else
        noffset = 0;

    Globals.CaretCurLine = a->drawnums[noffset];
    Globals.CaretCurPos = Globals.CaretAbsPos - maxlen * noffset;
}

void EDIT_DoBackspace(void)
{
    TextItem *a;

    if (Globals.TextList.first == NULL)
        return;

    for (a = Globals.TextList.first;
         a->drawnums[0] < Globals.CaretCurLine; a = a->next) {
            if (a == Globals.TextList.last)
                break;
    }
    if (a == Globals.TextList.last) {
        if (a->drawnums[a->noffsets - 1] < Globals.CaretCurLine) // Should not happen
            return;
    }
    else {
        if (a->drawnums[0] != Globals.CaretCurLine)
            a = a->prev;
    }

    if (Globals.CaretAbsPos > 0) {
        char *tmp;

        if ((tmp = malloc(a->str.len)) == NULL) {
            // TODO -- show some error message
            ExitProcess(1);
        }

        memcpy(tmp,
               a->str.data,
               Globals.CaretAbsPos - 1);
        memcpy(tmp + Globals.CaretAbsPos - 1,
               a->str.data + Globals.CaretAbsPos,
               a->str.len - Globals.CaretAbsPos + 1);

        free(a->str.data);
        a->str.data = tmp;
        a->str.len--;

        //printf("%s\n", tmp);
    }
    else {
        if (Globals.CaretAbsLine > 0) {
            char *tmp;

            if ((tmp = malloc(a->prev->str.len + a->str.len + 1)) == NULL) {
                // TODO -- show some error message
                ExitProcess(1);
            }

            memcpy(tmp,
                   a->prev->str.data,
                   a->prev->str.len);
            memcpy(tmp + a->prev->str.len,
                   a->str.data,
                   a->str.len + 1);

            Globals.CaretAbsLine--;
            Globals.CaretAbsPos = a->prev->str.len + 1;

            a->prev->next = a->next;
            if (a->next != NULL)
                a->next->prev = a->prev;
            else
                Globals.TextList.last = a->prev;

            free(a->prev->str.data);
            a->prev->str.data = tmp;
            a->prev->str.len += a->str.len;

            free(a->str.data);
            free(a->offsets);
            free(a->drawnums);
            free(a);

            EDIT_FixCaret();

            //printf("%s\n", tmp);
        }
    }

    EDIT_CountOffsets();
}

void EDIT_DoReturn(void)
{
    TextItem *a;

    if (Globals.TextList.first == NULL)
        return;

    for (a = Globals.TextList.first;
         a->drawnums[0] < Globals.CaretCurLine; a = a->next) {
            if (a == Globals.TextList.last)
                break;
    }
    if (a == Globals.TextList.last) {
        if (a->drawnums[a->noffsets - 1] < Globals.CaretCurLine) // Should not happen
            return;
    }
    else {
        if (a->drawnums[0] != Globals.CaretCurLine)
            a = a->prev;
    }

    char *tmp1, *tmp2;
    TextItem *b;

    if ((tmp1 = malloc(Globals.CaretAbsPos + 1)) == NULL) {
        // TODO -- show some error message
        ExitProcess(1);
    }
    if ((tmp2 = malloc(a->str.len - Globals.CaretAbsPos + 1)) == NULL) {
        // TODO -- show some error message
        ExitProcess(1);
    }

    memcpy(tmp1,
           a->str.data,
           Globals.CaretAbsPos);
    tmp1[Globals.CaretAbsPos] = '\0';

    memcpy(tmp2,
           a->str.data + Globals.CaretAbsPos,
           a->str.len - Globals.CaretAbsPos + 1);

    printf("a:%s\nb:%s\n", tmp1, tmp2);

    if ((b = malloc(sizeof(TextItem))) == NULL) {
        // TODO -- show some error message
        ExitProcess(1);
    }

    b->str.data = tmp2;
    b->str.len = a->str.len - Globals.CaretAbsPos;

    free(a->str.data);
    a->str.data = tmp1;
    a->str.len = Globals.CaretAbsPos;

    b->prev = a;
    b->next = a->next;
    a->next = b;

    if (b->next == NULL)
        Globals.TextList.last = b;

    Globals.CaretAbsLine++;
    Globals.CaretCurLine++;
    Globals.CaretAbsPos = 0;
    Globals.CaretCurPos = 0;

    EDIT_CountOffsets();
}

void EDIT_InsertCharacter(char c)
{
    TextItem *a;

    if (Globals.TextList.first == NULL)
        return;

    for (a = Globals.TextList.first;
         a->drawnums[0] < Globals.CaretCurLine; a = a->next) {
            if (a == Globals.TextList.last)
                break;
    }
    if (a == Globals.TextList.last) {
        if (a->drawnums[a->noffsets - 1] < Globals.CaretCurLine) // Should not happen
            return;
    }
    else {
        if (a->drawnums[0] != Globals.CaretCurLine)
            a = a->prev;
    }

    char *tmp;

    if ((tmp = malloc(a->str.len + 2)) == NULL) {
        // TODO -- show some error message
        ExitProcess(1);
    }

    memcpy(tmp,
           a->str.data,
           Globals.CaretAbsPos);
    tmp[Globals.CaretAbsPos] = c;
    memcpy(tmp + Globals.CaretAbsPos + 1,
           a->str.data + Globals.CaretAbsPos,
           a->str.len - Globals.CaretAbsPos + 1);

    free(a->str.data);
    a->str.data = tmp;
    a->str.len++;

    EDIT_CountOffsets();

    //printf("%s\n", tmp);
}
