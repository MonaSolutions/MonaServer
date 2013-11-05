/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
