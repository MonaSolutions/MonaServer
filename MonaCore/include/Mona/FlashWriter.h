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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Writer.h"
#include "Mona/AMF.h"
#include "Mona/AMFWriter.h"

namespace Mona {

class FlashWriter : public Writer, public virtual Object {
public:
	// For AMF response!
	
	bool					amf0;
	
	BinaryWriter&			writeRaw() { return write(AMF::RAW).packet; }
	AMFWriter&				writeMessage();
	AMFWriter&				writeInvocation(const char* name) { return writeInvocation(name,0); }

	AMFWriter&				writeAMFSuccess(const char* code, const std::string& description, bool withoutClosing = false) { return writeAMFState("_result", code, description, withoutClosing); }
	AMFWriter&				writeAMFStatus(const char* code, const std::string& description, bool withoutClosing = false) { return writeAMFState("onStatus", code, description, withoutClosing); }
	AMFWriter&				writeAMFError(const char* code, const std::string& description, bool withoutClosing = false) { return writeAMFState("_error", code, description, withoutClosing); }
	bool					writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties);

	AMFWriter&				writeAMFData(const std::string& name);

	void					writePing() { writeRaw().write16(0x0006).write32((UInt32)Time::Now()); }
	void					writePong(UInt32 pingTime) { writeRaw().write16(0x0007).write32(pingTime); }

	void					setCallbackHandle(double value) { _callbackHandle = value; _callbackHandleOnAbort = 0; }
	virtual void			clear() { _callbackHandle = _callbackHandleOnAbort; } // must erase the queueing messages (don't change the writer state)

protected:
	FlashWriter(State state,const PoolBuffers& poolBuffers);
	FlashWriter(FlashWriter& writer);

	virtual AMFWriter&		write(AMF::ContentType type,UInt32 time=0,const UInt8* data=NULL,UInt32 size=0)=0;
	AMFWriter&				writeInvocation(const char* name,double callback);
	AMFWriter&				writeAMFState(const char* name,const char* code,const std::string& description,bool withoutClosing=false);

	const PoolBuffers&		poolBuffers;
private:
	std::string				_onAudio;
	std::string				_onVideo;
	double					_callbackHandleOnAbort;
	double					_callbackHandle;
};



} // namespace Mona
