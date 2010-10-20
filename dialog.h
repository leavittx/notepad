/* Lev Panov, 2057/2, October 2010 */

#ifndef DIALOG_H
#define DIALOG_H

void DIALOG_FileNew(void);
void DIALOG_FileOpen(void);
bool DIALOG_FileSave(void);
bool DIALOG_FileSaveAs(void);
void DIALOG_FileExit(void);

void DIALOG_EditWrap(void);

int DIALOG_StringMsgBox(HWND hParent, int formatId, const char *String, DWORD dwFlags);

/* utility functions */
void ShowLastError(void);
void UpdateWindowCaption(void);
bool FileExists(const char *Filename);
bool DoCloseFile(void);
void DoOpenFile(const char *FileName);

#endif // DIALOG_H
