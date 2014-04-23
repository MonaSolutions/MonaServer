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
	double					callbackHandle;

	BinaryWriter&			writeRaw() { return write(AMF::RAW).packet; }
	AMFWriter&				writeMessage();
	AMFWriter&				writeInvocation(const std::string& name) { return writeInvocation(name,0); }

	AMFWriter&				writeAMFSuccess(const std::string& code, const std::string& description, bool withoutClosing = false) { return writeAMFState("_result", code, description, withoutClosing); }
	AMFWriter&				writeAMFStatus(const std::string& code, const std::string& description, bool withoutClosing = false) { return writeAMFState("onStatus", code, description, withoutClosing); }
	AMFWriter&				writeAMFError(const std::string& code, const std::string& description, bool withoutClosing = false) { return writeAMFState("_error", code, description, withoutClosing); }
	bool					writeMedia(MediaType type,UInt32 time,PacketReader& packet,Parameters& properties);

protected:
	FlashWriter(State state);
	FlashWriter(FlashWriter& writer);
	virtual ~FlashWriter();

	virtual AMFWriter&		write(AMF::ContentType type,UInt32 time=0,PacketReader* pPacket=NULL)=0;
	AMFWriter&				writeInvocation(const std::string& name,double callback);
	AMFWriter&				writeAMFState(const std::string& name,const std::string& code,const std::string& description,bool withoutClosing=false);
private:
	std::string		_onAudio;
	std::string		_onVideo;
};



} // namespace Mona
