#ifndef NOTEPAD_RES_H
#define NOTEPAD_RES_H

#include <windef.h>
#include <winuser.h>

#define MAIN_MENU               0x201

/* Commands */
#define CMD_NEW                 0x100
#define CMD_OPEN                0x101
#define CMD_SAVE                0x102
#define CMD_SAVE_AS             0x103
#define CMD_EXIT                0x108

#define CMD_CUT                 0x111
#define CMD_COPY                0x112
#define CMD_PASTE               0x113
#define CMD_DELETE              0x114
#define CMD_SELECT_ALL          0x116

#define CMD_WRAP                0x119
#define CMD_FONT                0x140

#define CMD_HELP_ABOUT_NOTEPAD  0x134

/* Strings */
#define STRING_NOTEPAD 0x170
#define STRING_ERROR 0x171
#define STRING_WARNING 0x172
#define STRING_INFO 0x173
#define STRING_UNTITLED 0x174
#define STRING_ALL_FILES 0x175
#define STRING_TEXT_FILES_TXT 0x176
#define STRING_TOOLARGE 0x177
#define STRING_NOTEXT 0x178
#define STRING_DOESNOTEXIST 0x179
#define STRING_NOTSAVED 0x17A

#define STRING_NOTFOUND 0x17B

/* Open/Save As dialog template */
#define IDD_OFN_TEMPLATE       0x190
#define IDC_OFN_ENCCOMBO       0x191

#endif // NOTEPAD_RES_H
