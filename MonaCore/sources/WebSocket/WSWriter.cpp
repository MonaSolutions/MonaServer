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

#include "Mona/WebSocket/WSWriter.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

WSWriter::WSWriter(TCPSession& session) : _dataType(MIME::UNKNOWN),_session(session),Writer(session.peer.connected ? OPENED : OPENING) {
	
}

void WSWriter::close(Int32 code) {
	if (code < 0)
		return;
	if (code == 0)
		code = WS::CODE_NORMAL_CLOSE;
	write(WS::TYPE_CLOSE, NULL, (UInt32)code);
	Writer::close(code);
	_session.kill(code);
}


DataWriter& WSWriter::newDataWriter(bool modeRaw) {
	pack();
	WSSender* pSender = new WSSender(_session.invoker.poolBuffers,modeRaw);
	_senders.emplace_back(pSender);
	// expect place for header
	DataWriter& writer(pSender->writer());
	writer.clear(10);
	return writer;
}

void WSWriter::pack() {
	if(_senders.empty())
		return;
	WSSender& sender = *_senders.back();
	if(sender.packaged)
		return;
	PacketWriter& packet = sender.writer().packet;
	UInt32 size = packet.size()-10;
	packet.clip(10-WS::HeaderSize(size));
	BinaryWriter headerWriter(packet.data(),packet.size());
	packet.clear(WS::WriteHeader(sender.type,size,headerWriter)+size);
	sender.packaged = true;
}

bool WSWriter::flush() {
	if(_senders.empty())
		return false;
	pack();
	Exception ex;
	for (shared_ptr<WSSender>& pSender : _senders) {
		_session.dumpResponse(pSender->data(),pSender->size());
		EXCEPTION_TO_LOG(_session.send<WSSender>(ex, qos(), pSender), "WSSender flush");
	}
	_senders.clear();
	return true;
}

void WSWriter::write(UInt8 type,const UInt8* data,UInt32 size) {
	if(state()==CLOSED)
		return;
	pack();
	WSSender* pSender = new WSSender(_session.invoker.poolBuffers,true);
	pSender->packaged = true;
	_senders.emplace_back(pSender);
	BinaryWriter& writer(pSender->writer().packet);
	if(type==WS::TYPE_CLOSE) {
		// here size is the code!
		if(size>0) {
			WS::WriteHeader((WS::MessageType)type,2,writer);
			writer.write16(size);
		} else
			WS::WriteHeader((WS::MessageType)type,0,writer);
		return;
	}
	WS::WriteHeader((WS::MessageType)type,size,writer);
	writer.write(data,size);
}



DataWriter& WSWriter::writeInvocation(const char* name) {
	if(state()==CLOSED)
        return DataWriter::Null;
	DataWriter& invocation(newDataWriter());
	invocation.writeString(name,strlen(name));
	return invocation;
}

DataWriter& WSWriter::writeResponse(UInt8 type) {
	 if (type == 0)
		 return writeMessage(); 
	if(state()==CLOSED)
        return DataWriter::Null;
	return newDataWriter(true);
}

DataWriter& WSWriter::writeMessage() {
	if(state()==CLOSED)
        return DataWriter::Null;
	return newDataWriter();
}



bool WSWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties) {
	if(state()==CLOSED)
		return true;
	switch(type) {
		case START:
			if (time==DATA)
				writeInvocation("__publishing").writeString((const char*)packet.current(),packet.available());
			break;
		case STOP:
			if (time==DATA)
				writeInvocation("__unpublishing").writeString((const char*)packet.current(),packet.available());
			break;
		case INIT: {
			string dataType;
			if (properties.getString("dataType", dataType) && (_dataType = MIME::DataType(dataType.c_str())))
				_dataType = MIME::UNKNOWN;
			break;
		}
		case DATA: {
			if (_dataType) { // conversion?
				MIME::Type dataType((MIME::Type)(time >> 8));
				if (_dataType != dataType) {
					unique_ptr<DataReader> pReader;
					if (MIME::CreateDataReader(dataType, packet, _session.invoker.poolBuffers, pReader))
						pReader->read(newDataWriter()); // to JSON
				}
				break;
			}
			// raw
			newDataWriter(true).packet.write(packet.current(), packet.available());
			break;
		}
		default:
			return Writer::writeMedia(type,time,packet,properties);
	}
	return true;
}


} // namespace Mona
