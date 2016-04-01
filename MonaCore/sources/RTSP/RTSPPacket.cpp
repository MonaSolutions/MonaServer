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

#include "Mona/RTSP/RTSPPacket.h"
#include "Mona/Util.h"

using namespace std;


namespace Mona {


RTSPPacket::RTSPPacket(const string& rootPath) :
	_data(NULL),_size(0),cSeq(0),trackID(0),
	command(RTSP::COMMAND_UNKNOWN),
	version(0) {

}

void RTSPPacket::parseHeader(Exception& ex,const char* key, const char* value) {
	if (String::ICompare(key,"CSeq")==0) {
		cSeq = String::ToNumber(ex, cSeq, value);
	}
}
	
UInt32 RTSPPacket::build(Exception& ex,UInt8* data,UInt32 size) {
	if (_data)
		return 0;
	exception.set(Exception::NIL);

	/// read data
	ReadingStep			step(CMD);
	UInt8*				current(data);
	const UInt8*		end(current+size);
	const char*			signifiant(NULL);
	const char*			key(NULL);

	// headers

	for (; current <= end;++current) {

		if (memcmp(current, EXPAND("\r\n")) == 0 || memcmp(current, EXPAND("\0\n")) == 0) {

			if (!ex && signifiant) {
				// KEY = VALUE
				UInt8* endValue(current);
				while (isblank(*--endValue));
				*(endValue+1) = 0;
				if (!key) { // version case!
					String::ToNumber(signifiant+5, version);
				} else {
					headers[key] = signifiant;
					parseHeader(ex,key,signifiant);
					key = NULL;
				}
			}
			step = LEFT;
			current += 2;
			signifiant = (const char*)current;
			++current; // here no continue, the "\r\n" check is not required again
		}

		if (ex)
			continue; // try to go to "\r\n\r\n"

		// http header, byte by byte
		UInt8 byte = *current;

		if ((step == LEFT || step == CMD || step == PATH) && (isspace(byte) || byte==0)) {
			if (step == CMD) {
				if ((command = RTSP::ParseCommand(ex, signifiant)) == RTSP::COMMAND_UNKNOWN) {
					exception = ex;
					continue;
				}
				if(command == RTSP::COMMAND_DESCRIBE) // Only for DESCRIBE : content = application/sdp
					responseType = HTTP::CONTENT_APPLICATON;
				signifiant = NULL;
				step = PATH;
			} else if (step == PATH) {
				// parse query
				*current = 0;
				size_t filePos = Util::UnpackUrl(signifiant, path,query);
				url.assign(signifiant);

				// Get trackID
				if(filePos != string::npos && path.size() > 9 && String::ICompare(path.substr(filePos),"trackID=",8)==0) {
					trackID = String::ToNumber<UInt8>(ex, 0, path.substr(filePos+8));
					path.erase(filePos - 1); // remove trackID part
					filePos = path.find_last_of('/');
					if (filePos != string::npos)
						filePos++;
				}

				// Record file directory and path
				file.setPath(path);
				if (filePos != string::npos)
					path.erase(filePos - 1);
				else
					file.makeFolder();
				signifiant = NULL;
				step = VERSION;
			} else
				++signifiant; // for trim begin of key or value
		} else if (step > PATH && !key && (byte == ':' || byte == 0)) {
			// KEY
			key = signifiant;
			step = LEFT;
			UInt8* endValue(current);
			while (isblank(*--endValue));
			*(endValue+1) = 0;
			signifiant = (const char*)current + 1;
		} else if (step == CMD || step == PATH || step == VERSION) {
			if (!signifiant)
				signifiant = (const char*)current;
			if (step == CMD && (current-data)>13) // not a RTSP valid command, consumes all
				exception = ex.set(Exception::PROTOCOL, "invalid RTSP command");
		} else
			step = RIGHT;
	}

	_data = data;
	return _size = current - data;
}


} // namespace Mona
