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
#include "Mona/MIME.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {


FlashWriter::FlashWriter(State state,const PoolBuffers& poolBuffers) : poolBuffers(poolBuffers),_callbackHandleOnAbort(0),_callbackHandle(0),Writer(state),amf0(false) {
}

FlashWriter::FlashWriter(FlashWriter& writer) : poolBuffers(writer.poolBuffers),_callbackHandle(writer._callbackHandle),_callbackHandleOnAbort(0),Writer(writer),amf0(writer.amf0) {
	writer._callbackHandle = 0;
}

AMFWriter& FlashWriter::writeMessage() {
	AMFWriter& writer(writeInvocation("_result",_callbackHandleOnAbort = _callbackHandle));
	_callbackHandle = 0;
	return writer;
}

AMFWriter& FlashWriter::writeInvocation(const char* name, double callback) {
	AMFWriter& writer = write(AMF::INVOCATION);
	BinaryWriter& packet = writer.packet;
	packet.write8(AMF_STRING).write16((UInt16)strlen(name)).write(name);
	packet.write8(AMF_NUMBER).writeNumber<double>(callback);
	packet.write8(AMF_NULL); // for RTMP compatibility! (requiere it)
	writer.amf0 = amf0;
	return writer;
}

AMFWriter& FlashWriter::writeAMFState(const char* name,const char* code,const string& description,bool withoutClosing) {
	AMFWriter& writer = (AMFWriter&)writeInvocation(name,_callbackHandleOnAbort = _callbackHandle);
	_callbackHandle = 0;
	writer.amf0=true;
	writer.beginObject();
	if(strcmp(name,"_error")==0)
		writer.writeStringProperty("level","error");
	else
		writer.writeStringProperty("level","status");
	writer.writeStringProperty("code",code);
	writer.writeStringProperty("description", description);
	writer.amf0 = amf0;
	if(!withoutClosing)
		writer.endObject();
	return writer;
}

AMFWriter& FlashWriter::writeAMFData(const string& name) {
	AMFWriter& writer(write(AMF::DATA));
	writer.amf0 = true;
	writer.writeString(name.data(),name.size());
	writer.amf0 = false;
	return writer;
}

bool FlashWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties) {
	
	switch(type) {
		case INIT:
			if (time == AUDIO) {
				_onAudio.clear();
				if (properties.getString("onAudio", _onAudio) && amf0) {
					WARN("Impossible to handle onAudio properties on a AMF0 stream (no ByteArray support)")
					_onAudio.clear();
				}
			} else if (time == VIDEO) {
				_onVideo.clear();
				if (properties.getString("onVideo", _onVideo) && amf0) {
					WARN("Impossible to handle onVideo properties on a AMF0 stream (no ByteArray support)")
					_onVideo.clear();
				}
			}
			break;
		case START:
			if (time==DATA)
				writeAMFStatus("NetStream.Play.PublishNotify",string(STR packet.current(),packet.available()) + " is now published");
			break;
		case STOP:
			if (time==DATA)
				writeAMFStatus("NetStream.Play.UnpublishNotify",string(STR packet.current(),packet.available()) + " is now unpublished");
			break;
		case AUDIO:
			if (!_onAudio.empty()) {
				AMFWriter& writer(writeAMFData(_onAudio));
				writer.writeNumber(time);
				writer.writeBytes(packet.current(),packet.available());
			} else
				write(AMF::AUDIO,time,packet.current(),packet.available());
			break;
		case VIDEO:
			if (!_onVideo.empty()) {
				AMFWriter& writer(writeAMFData(_onVideo));
				writer.writeNumber(time);
				writer.writeBytes(packet.current(),packet.available());
				
			} else
				write(AMF::VIDEO,time,packet.current(),packet.available());
			break;
		case DATA: {
			// convert to AMF ?
			MIME::Type dataType((MIME::Type)(time >> 8));
			if (dataType!=MIME::AMF) {
				unique_ptr<DataReader> pReader;
				if (!MIME::CreateDataReader(dataType, packet,poolBuffers, pReader)) {
					ERROR("Impossible to convert streaming ", dataType, " data to AMF, data ignored")
					break;
				}
				AMFWriter& writer(write(AMF::DATA, 0));
				if (DataReader::STRING == pReader->nextType()) {
					// Write the handler name in AMF0!
					writer.amf0 = true;
					pReader->read(writer, 1);
					writer.amf0 = false;
				}
				pReader->read(writer); // to AMF
				break;
			}
			write(AMF::DATA, 0, packet.current(),packet.available());
			break;
		}
		default:
			return Writer::writeMedia(type,time,packet,properties);
	}
	return true;
}

} // namespace Mona
