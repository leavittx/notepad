/* Lev Panov, 2057/2, October 2010 */

#ifndef DIALOG_H
#define DIALOG_H

#include <windows.h>

/* Dialog functions */
void DIALOG_FileNew(void);    // New file dialog
void DIALOG_FileOpen(void);   // Open file dialog
bool DIALOG_FileSave(void);   // Save file dialog
bool DIALOG_FileSaveAs(void); // Save file as dialog
void DIALOG_FileExit(void);   // File exit dialog

void DIALOG_EditWrap(void);   // Toggle wrap mode dialog

int DIALOG_StringMsgBox(HWND hParent, int formatId, const char *String, DWORD dwFlags); // Format and show a message in a message box

/* Utility functions */
void ShowLastError(void);              // Show last windows error
void UpdateWindowCaption(void);        // Update window caption
bool FileExists(const char *Filename); // Check if file exists
bool DoCloseFile(void);                // Close file
void DoOpenFile(const char *FileName); // Open file

#endif // DIALOG_H
