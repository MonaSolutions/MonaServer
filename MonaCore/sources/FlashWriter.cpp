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


using namespace std;


namespace Mona {


FlashWriter::FlashWriter(State state) : _callbackHandleOnAbort(0),_callbackHandle(0),Writer(state),amf0(false) {
}

FlashWriter::FlashWriter(FlashWriter& writer) : _callbackHandle(writer._callbackHandle),_callbackHandleOnAbort(0),Writer(writer),amf0(writer.amf0) {
	writer._callbackHandle = 0;
}

FlashWriter::~FlashWriter() {
}


AMFWriter& FlashWriter::writeMessage() {
	AMFWriter& writer(writeInvocation("_result",_callbackHandleOnAbort = _callbackHandle));
	_callbackHandle = 0;
	return writer;
}

AMFWriter& FlashWriter::writeInvocation(const std::string& name,double callback) {
	AMFWriter& writer = write(AMF::INVOCATION);
	BinaryWriter& packet = writer.packet;
	packet.write8(AMF_STRING);packet.writeString16(name);
	packet.write8(AMF_NUMBER);
	packet.writeNumber<double>(callback);
	packet.write8(AMF_NULL); // for RTMP compatibility! (requiere it)
	writer.amf0 = amf0;
	return writer;
}

AMFWriter& FlashWriter::writeAMFState(const string& name,const string& code,const string& description,bool withoutClosing) {
	AMFWriter& writer = (AMFWriter&)writeInvocation(name,_callbackHandleOnAbort = _callbackHandle);
	_callbackHandle = 0;
	writer.amf0=true;
	writer.beginObject();
	if(name=="_error")
		writer.writeStringProperty("level","error");
	else
		writer.writeStringProperty("level","status");
	writer.writeStringProperty("code",code);
	writer.writeStringProperty("description",description);
	writer.amf0 = amf0;
	if(!withoutClosing)
		writer.endObject();
	return writer;
}


bool FlashWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,Parameters& properties) {
	
	switch(type) {
		case START:
			writeAMFStatus("NetStream.Play.PublishNotify",string((const char*)packet.current(),packet.available()) + " is now published");
			break;
		case STOP:
			writeAMFStatus("NetStream.Play.UnpublishNotify",string((const char*)packet.current(),packet.available()) +" is now unpublished");
			break;
		case AUDIO:
			if (!_onAudio.empty()) {
				AMFWriter& writer(write(AMF::DATA));
				writer.amf0 = true;
				writer.writeString(_onAudio);
				writer.amf0 = false;
				writer.writeNumber(time);
				writer.writeBytes(packet.current(),packet.available());
			} else
				write(AMF::AUDIO,time,&packet);
			break;
		case VIDEO:
			if (!_onVideo.empty()) {
				AMFWriter& writer(write(AMF::DATA));
				writer.amf0 = true;
				writer.writeString(_onVideo);
				writer.amf0 = false;
				writer.writeNumber(time);
				writer.writeBytes(packet.current(),packet.available());
				
			} else
				write(AMF::VIDEO,time,&packet);
			break;
		case DATA:
			write(AMF::DATA,time,&packet);
			break;
		case INIT:
			_onAudio.clear();
			_onVideo.clear();
			if (!amf0) {
				properties.getString("onAudio",_onAudio);
				properties.getString("onVideo",_onVideo);
			} else
				WARN("Impossible to handle onAudio and onVideo properties on a AMF0 stream")
			break;
	}
	return true;
}

} // namespace Mona
