#ifndef DIALOG_H
#define DIALOG_H

VOID DIALOG_FileNew(VOID);
VOID DIALOG_FileOpen(VOID);
BOOL DIALOG_FileSave(VOID);
BOOL DIALOG_FileSaveAs(VOID);
VOID DIALOG_FilePrint(VOID);
VOID DIALOG_FilePageSetup(VOID);
VOID DIALOG_FilePrinterSetup(VOID);
VOID DIALOG_FileExit(VOID);

VOID DIALOG_EditUndo(VOID);
VOID DIALOG_EditCut(VOID);
VOID DIALOG_EditCopy(VOID);
VOID DIALOG_EditPaste(VOID);
VOID DIALOG_EditDelete(VOID);
VOID DIALOG_EditSelectAll(VOID);
VOID DIALOG_EditTimeDate(VOID);
VOID DIALOG_EditWrap(VOID);

VOID DIALOG_Search(VOID);
VOID DIALOG_SearchNext(VOID);
VOID DIALOG_Replace(VOID);

VOID DIALOG_SelectFont(VOID);

VOID DIALOG_HelpContents(VOID);
VOID DIALOG_HelpSearch(VOID);
VOID DIALOG_HelpHelp(VOID);
VOID DIALOG_HelpAboutNotepad(VOID);

VOID DIALOG_TimeDate(VOID);
int DIALOG_StringMsgBox(HWND hParent, int formatId, LPCWSTR szString, DWORD dwFlags);

/* utility functions */
VOID ShowLastError(void);
void UpdateWindowCaption(void);
BOOL FileExists(LPCWSTR szFilename);
BOOL DoCloseFile(void);
void DoOpenFile(LPCWSTR szFileName, ENCODING enc);

#endif // DIALOG_H
