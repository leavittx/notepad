#ifndef DIALOG_H
#define DIALOG_H

VOID DIALOG_FileNew(VOID);
VOID DIALOG_FileOpen(VOID);
BOOL DIALOG_FileSave(VOID);
BOOL DIALOG_FileSaveAs(VOID);
VOID DIALOG_FileExit(VOID);

VOID DIALOG_EditUndo(VOID);
VOID DIALOG_EditCut(VOID);
VOID DIALOG_EditCopy(VOID);
VOID DIALOG_EditPaste(VOID);
VOID DIALOG_EditDelete(VOID);
VOID DIALOG_EditSelectAll(VOID);
VOID DIALOG_EditWrap(VOID);

VOID DIALOG_SelectFont(VOID);

VOID DIALOG_HelpAboutNotepad(VOID);

int DIALOG_StringMsgBox(HWND hParent, int formatId, LPCWSTR szString, DWORD dwFlags);

/* utility functions */
VOID ShowLastError(void);
void UpdateWindowCaption(void);
BOOL FileExists(LPCWSTR szFilename);
BOOL DoCloseFile(void);
void DoOpenFile(LPCWSTR szFileName, ENCODING enc);

#endif // DIALOG_H
