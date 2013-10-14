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

#include "Mona/Logs.h"
#include "Mona/Util.h"

using namespace std;
using namespace Poco;

namespace Mona {

Logger*			Logs::_PLogger(NULL);
Logs::DumpMode	Logs::_DumpMode(NOTHING);
UInt8			Logs::_Level(Logger::PRIO_INFO); // default log level


void Logs::Dump(const UInt8* data,UInt32 size,const char* header) {
	if(!GetLogger())
		return;
	vector<UInt8> out;
	Util::Dump(data,size,out,header);
	if(out.size()>0)
		GetLogger()->dumpHandler(&out[0],out.size());
}


} // namespace Mona
