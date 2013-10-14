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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Writer.h"
#include "Mona/AMF.h"
#include "Mona/AMFWriter.h"

namespace Mona {

class FlashWriter : public Writer {
public:
	// For AMF response!
	double					callbackHandle;

	BinaryWriter&			writeRaw();
	AMFWriter&				writeMessage();
	AMFWriter&				writeInvocation(const std::string& name);
	AMFWriter&				writeAMFSuccess(const std::string& code,const std::string& description,bool withoutClosing=false);
	AMFWriter&				writeAMFStatus(const std::string& code,const std::string& description,bool withoutClosing=false);
	AMFWriter&				writeAMFError(const std::string& code,const std::string& description,bool withoutClosing=false);
	bool					writeMedia(MediaType type,Mona::UInt32 time,MemoryReader& data);


	static Mona::AMFWriterNull	AMFWriterNull;
protected:
	FlashWriter(WriterHandler* pHandler=NULL);
	FlashWriter(FlashWriter& writer);
	virtual ~FlashWriter();

	virtual AMFWriter&		write(AMF::ContentType type,Mona::UInt32 time=0,MemoryReader* pData=NULL)=0;

	AMFWriter&				writeAMFState(const std::string& name,const std::string& code,const std::string& description,bool withoutClosing=false);
};

inline BinaryWriter& FlashWriter::writeRaw() {
	return write(AMF::RAW).writer;
}

inline AMFWriter& FlashWriter::writeMessage() {
	return writeInvocation("_result");
}

inline AMFWriter& FlashWriter::writeAMFSuccess(const std::string& code,const std::string& description,bool withoutClosing) {
	return writeAMFState("_result",code,description,withoutClosing);
}
inline AMFWriter& FlashWriter::writeAMFStatus(const  std::string& code,const std::string& description,bool withoutClosing) {
	return writeAMFState("onStatus",code,description,withoutClosing);
}
inline AMFWriter& FlashWriter::writeAMFError(const  std::string& code,const std::string& description,bool withoutClosing) {
	return writeAMFState("_error",code,description,withoutClosing);
}



} // namespace Mona
