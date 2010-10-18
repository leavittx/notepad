/* Lev Panov, 2057/2, October 2010 */

#ifndef DIALOG_H
#define DIALOG_H

VOID DIALOG_FileNew(VOID);
VOID DIALOG_FileOpen(VOID);
BOOL DIALOG_FileSave(VOID);
BOOL DIALOG_FileSaveAs(VOID);
VOID DIALOG_FileExit(VOID);

VOID DIALOG_EditWrap(VOID);
VOID DIALOG_SelectFont(VOID);

int DIALOG_StringMsgBox(HWND hParent, int formatId, LPCSTR szString, DWORD dwFlags);

/* utility functions */
VOID ShowLastError(void);
void UpdateWindowCaption(void);
BOOL FileExists(LPCSTR szFilename);
BOOL DoCloseFile(void);
void DoOpenFile(LPCSTR szFileName);

#endif // DIALOG_H
