/*
Copyright 2014 Mona
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

This file is a part of Mona.
*/

#include "Mona/Mona.h"

namespace Mona {

const UInt16 ASCII::_CharacterTypes[128] =  {
	/* 00 . */ CONTROL,
	/* 01 . */ CONTROL,
	/* 02 . */ CONTROL,
	/* 03 . */ CONTROL,
	/* 04 . */ CONTROL,
	/* 05 . */ CONTROL,
	/* 06 . */ CONTROL,
	/* 07 . */ CONTROL,
	/* 08 . */ CONTROL,
	/* 09 . */ CONTROL | SPACE | BLANK,
	/* 0a . */ CONTROL | SPACE,
	/* 0b . */ CONTROL | SPACE,
	/* 0c . */ CONTROL | SPACE,
	/* 0d . */ CONTROL | SPACE,
	/* 0e . */ CONTROL,
	/* 0f . */ CONTROL,
	/* 10 . */ CONTROL,
	/* 11 . */ CONTROL,
	/* 12 . */ CONTROL,
	/* 13 . */ CONTROL,
	/* 14 . */ CONTROL,
	/* 15 . */ CONTROL,
	/* 16 . */ CONTROL,
	/* 17 . */ CONTROL,
	/* 18 . */ CONTROL,
	/* 19 . */ CONTROL,
	/* 1a . */ CONTROL,
	/* 1b . */ CONTROL,
	/* 1c . */ CONTROL,
	/* 1d . */ CONTROL,
	/* 1e . */ CONTROL,
	/* 1f . */ CONTROL,
	/* 20   */ SPACE | BLANK | PRINT,
	/* 21 ! */ PUNCT | GRAPH | PRINT,
	/* 22 " */ PUNCT | GRAPH | PRINT,
	/* 23 # */ PUNCT | GRAPH | PRINT,
	/* 24 $ */ PUNCT | GRAPH | PRINT,
	/* 25 % */ PUNCT | GRAPH | PRINT,
	/* 26 & */ PUNCT | GRAPH | PRINT,
	/* 27 ' */ PUNCT | GRAPH | PRINT,
	/* 28 ( */ PUNCT | GRAPH | PRINT,
	/* 29 ) */ PUNCT | GRAPH | PRINT,
	/* 2a * */ PUNCT | GRAPH | PRINT,
	/* 2b + */ PUNCT | GRAPH | PRINT,
	/* 2c , */ PUNCT | GRAPH | PRINT,
	/* 2d - */ PUNCT | GRAPH | PRINT,
	/* 2e . */ PUNCT | GRAPH | PRINT,
	/* 2f / */ PUNCT | GRAPH | PRINT,
	/* 30 0 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 31 1 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 32 2 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 33 3 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 34 4 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 35 5 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 36 6 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 37 7 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 38 8 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 39 9 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
	/* 3a : */ PUNCT | GRAPH | PRINT,
	/* 3b ; */ PUNCT | GRAPH | PRINT,
	/* 3c < */ PUNCT | GRAPH | PRINT,
	/* 3d = */ PUNCT | GRAPH | PRINT,
	/* 3e > */ PUNCT | GRAPH | PRINT,
	/* 3f ? */ PUNCT | GRAPH | PRINT,
	/* 40 @ */ PUNCT | GRAPH | PRINT,
	/* 41 A */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
	/* 42 B */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
	/* 43 C */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
	/* 44 D */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
	/* 45 E */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
	/* 46 F */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
	/* 47 G */ ALPHA | UPPER | GRAPH | PRINT,
	/* 48 H */ ALPHA | UPPER | GRAPH | PRINT,
	/* 49 I */ ALPHA | UPPER | GRAPH | PRINT,
	/* 4a J */ ALPHA | UPPER | GRAPH | PRINT,
	/* 4b K */ ALPHA | UPPER | GRAPH | PRINT,
	/* 4c L */ ALPHA | UPPER | GRAPH | PRINT,
	/* 4d M */ ALPHA | UPPER | GRAPH | PRINT,
	/* 4e N */ ALPHA | UPPER | GRAPH | PRINT,
	/* 4f O */ ALPHA | UPPER | GRAPH | PRINT,
	/* 50 P */ ALPHA | UPPER | GRAPH | PRINT,
	/* 51 Q */ ALPHA | UPPER | GRAPH | PRINT,
	/* 52 R */ ALPHA | UPPER | GRAPH | PRINT,
	/* 53 S */ ALPHA | UPPER | GRAPH | PRINT,
	/* 54 T */ ALPHA | UPPER | GRAPH | PRINT,
	/* 55 U */ ALPHA | UPPER | GRAPH | PRINT,
	/* 56 V */ ALPHA | UPPER | GRAPH | PRINT,
	/* 57 W */ ALPHA | UPPER | GRAPH | PRINT,
	/* 58 X */ ALPHA | UPPER | GRAPH | PRINT,
	/* 59 Y */ ALPHA | UPPER | GRAPH | PRINT,
	/* 5a Z */ ALPHA | UPPER | GRAPH | PRINT,
	/* 5b [ */ PUNCT | GRAPH | PRINT,
	/* 5c \ */ PUNCT | GRAPH | PRINT,
	/* 5d ] */ PUNCT | GRAPH | PRINT,
	/* 5e ^ */ PUNCT | GRAPH | PRINT,
	/* 5f _ */ PUNCT | GRAPH | PRINT,
	/* 60 ` */ PUNCT | GRAPH | PRINT,
	/* 61 a */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
	/* 62 b */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
	/* 63 c */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
	/* 64 d */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
	/* 65 e */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
	/* 66 f */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
	/* 67 g */ ALPHA | LOWER | GRAPH | PRINT,
	/* 68 h */ ALPHA | LOWER | GRAPH | PRINT,
	/* 69 i */ ALPHA | LOWER | GRAPH | PRINT,
	/* 6a j */ ALPHA | LOWER | GRAPH | PRINT,
	/* 6b k */ ALPHA | LOWER | GRAPH | PRINT,
	/* 6c l */ ALPHA | LOWER | GRAPH | PRINT,
	/* 6d m */ ALPHA | LOWER | GRAPH | PRINT,
	/* 6e n */ ALPHA | LOWER | GRAPH | PRINT,
	/* 6f o */ ALPHA | LOWER | GRAPH | PRINT,
	/* 70 p */ ALPHA | LOWER | GRAPH | PRINT,
	/* 71 q */ ALPHA | LOWER | GRAPH | PRINT,
	/* 72 r */ ALPHA | LOWER | GRAPH | PRINT,
	/* 73 s */ ALPHA | LOWER | GRAPH | PRINT,
	/* 74 t */ ALPHA | LOWER | GRAPH | PRINT,
	/* 75 u */ ALPHA | LOWER | GRAPH | PRINT,
	/* 76 v */ ALPHA | LOWER | GRAPH | PRINT,
	/* 77 w */ ALPHA | LOWER | GRAPH | PRINT,
	/* 78 x */ ALPHA | LOWER | GRAPH | PRINT,
	/* 79 y */ ALPHA | LOWER | GRAPH | PRINT,
	/* 7a z */ ALPHA | LOWER | GRAPH | PRINT,
	/* 7b { */ PUNCT | GRAPH | PRINT,
	/* 7c | */ PUNCT | GRAPH | PRINT,
	/* 7d } */ PUNCT | GRAPH | PRINT,
	/* 7e ~ */ PUNCT | GRAPH | PRINT,
	/* 7f . */ CONTROL
};



#if defined(_WIN32) && defined(_DEBUG)

#include <windows.h>

#define FALSE   0
#define TRUE    1


_CRT_REPORT_HOOK prevHook;
 
int reportingHook(int reportType, char* userMessage, int* retVal) {
  // This function is called several times for each memory leak.
  // Each time a part of the error message is supplied.
  // This holds number of subsequent detail messages after
  // a leak was reported
  const int numFollowupDebugMsgParts = 2;
  static bool ignoreMessage = false;
  static int debugMsgPartsCount = numFollowupDebugMsgParts;
  static bool firstMessage = true;

  if( strncmp(userMessage,"Detected memory leaks!\n", 10)==0) {
	ignoreMessage = true;
	return TRUE;
  } else if(strncmp(userMessage,"Dumping objects ->\n", 10)==0) {
	  return TRUE;
  } else if (ignoreMessage) {

    // check if the memory leak reporting ends
    if (strncmp(userMessage,"Object dump complete.\n", 10) == 0) {
		  _CrtSetReportHook(prevHook);
		  ignoreMessage = false;
    }

	if(debugMsgPartsCount++ < numFollowupDebugMsgParts)
		// give it back to _CrtDbgReport() to be printed to the console
		return FALSE;

 
    // something from our own code?
	if(strstr(userMessage, ".cpp") || strstr(userMessage, ".h") || strstr(userMessage, ".c")) {
		if(firstMessage) {
			OutputDebugStringA("Detected memory leaks!\nDumping objects ->\n");
			firstMessage = false;
		}
		
		debugMsgPartsCount = 0;
      // give it back to _CrtDbgReport() to be printed to the console
       return FALSE;
    } else
		return TRUE;

 } else
    // give it back to _CrtDbgReport() to be printed to the console
    return FALSE;

  
};
 
void DetectMemoryLeak() {
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  //change the report function to only report memory leaks from program code
  prevHook = _CrtSetReportHook(reportingHook);
}

#else
void DetectMemoryLeak() {}
#endif

} // namespace Mona
