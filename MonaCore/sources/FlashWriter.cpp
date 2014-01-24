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

#include "Mona/FlashWriter.h"
#include "Mona/AMF.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"
#include <cstring>

using namespace std;


namespace Mona {


FlashWriter::FlashWriter(WriterHandler* pHandler) : callbackHandle(0),Writer(pHandler),amf0Preference(false) {
}

FlashWriter::FlashWriter(FlashWriter& writer) : callbackHandle(writer.callbackHandle),Writer(writer),amf0Preference(false) {
}

FlashWriter::~FlashWriter() {
}

AMFWriter& FlashWriter::writeInvocation(const string& name) {
	AMFWriter& writer = write(AMF::INVOCATION);
	BinaryWriter& packet = writer.packet;
	packet.write8(AMF_STRING);packet.writeString16(name);
	packet.write8(AMF_NUMBER);
	packet.writeNumber<double>(callbackHandle);
	packet.write8(AMF_NULL); // for RTMP compatibility! (requiere it)
	writer.amf0Preference = amf0Preference;
	return writer;
}

AMFWriter& FlashWriter::writeAMFState(const string& name,const string& code,const string& description,bool withoutClosing) {
	AMFWriter& writer = (AMFWriter&)writeInvocation(name);
	writer.amf0Preference=true;
	writer.beginObject();
	if(name=="_error")
		writer.writeStringProperty("level","error");
	else
		writer.writeStringProperty("level","status");
	writer.writeStringProperty("code",code);
	writer.writeStringProperty("description",description);
	writer.amf0Preference = amf0Preference;
	if(!withoutClosing)
		writer.endObject();
	return writer;
}


bool FlashWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet) {
	switch(type) {
		case START:
			writeAMFStatus("Play.PublishNotify",string((const char*)packet.current(),packet.available()) + " is now published");
			break;
		case STOP:
			writeAMFStatus("Play.UnpublishNotify",string((const char*)packet.current(),packet.available()) +" is now unpublished");
			break;
		case AUDIO:
			write(AMF::AUDIO,time,&packet);
			break;
		case VIDEO:
			write(AMF::VIDEO,time,&packet);
			break;
		case DATA:
			write(AMF::DATA,time,&packet);
			break;
	}
	return true;
}

} // namespace Mona
