/* Lev Panov, 2057/2, October 2010 */

#ifndef DIALOG_H
#define DIALOG_H

void DIALOG_FileNew(void);
void DIALOG_FileOpen(void);
BOOL DIALOG_FileSave(void);
BOOL DIALOG_FileSaveAs(void);
void DIALOG_FileExit(void);

void DIALOG_EditWrap(void);

int DIALOG_StringMsgBox(HWND hParent, int formatId, LPCSTR szString, DWORD dwFlags);

/* utility functions */
void ShowLastError(void);
void UpdateWindowCaption(void);
BOOL FileExists(LPCSTR szFilename);
BOOL DoCloseFile(void);
void DoOpenFile(LPCSTR szFileName);

#endif // DIALOG_H
