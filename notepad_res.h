/* Lev Panov, 2057/2, October 2010 */

#ifndef NOTEPAD_RES_H
#define NOTEPAD_RES_H

#include <windef.h>
#include <winuser.h>

#define MAIN_MENU               0x201
#define ID_ACCEL                0x203

/* Commands */
#define CMD_NEW                 0x100
#define CMD_OPEN                0x101
#define CMD_SAVE                0x102
#define CMD_SAVE_AS             0x103
#define CMD_EXIT                0x108

#define CMD_WRAP                0x119

/* Strings */
#define STRING_NOTEPAD 0x170
#define STRING_ERROR 0x171

#define STRING_UNTITLED 0x174

#define STRING_ALL_FILES 0x175
#define STRING_TEXT_FILES_TXT 0x176

#define STRING_DOESNOTEXIST 0x179
#define STRING_NOTSAVED 0x17A

#define STRING_NOTFOUND 0x17B

#endif // NOTEPAD_RES_H
