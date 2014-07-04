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

#include "Mona/FlashMainStream.h"
#include "Mona/Invoker.h"
#include "Mona/Util.h"
#include "Mona/Exceptions.h"
#include "Mona/Logs.h"
#include <openssl/evp.h>


using namespace std;


namespace Mona {


FlashMainStream::FlashMainStream(Invoker& invoker,Peer& peer) : FlashStream(0,invoker,peer),_pGroup(NULL) {
	
}

FlashMainStream::~FlashMainStream() {
	if(_pGroup)
		peer.unjoinGroup(*_pGroup);
	// delete stream index remaining (which have not had time to send a 'destroyStream' message)
	for(auto& it : _streams)
		invoker.destroyFlashStream(it.first);
}


FlashStream* FlashMainStream::stream(UInt32 id) {
	auto it = _streams.find(id);
	if (it == _streams.end())
		return NULL;
	return it->second.get();
}

void FlashMainStream::messageHandler(const string& name,AMFReader& message,FlashWriter& writer) {
	if(name=="connect") {
		message.stopReferencing();
		AMFReader::Type type;
		string name;
		bool external(false);
		if(message.readObject(name,external) && !external) {
			
			auto& properties(peer.properties());
			while((type=message.readItem(name))!=AMFReader::END) {
				switch(type) {
					case AMFReader::BOOLEAN:
						properties.setBool(name,message.readBoolean());
						break;
					case AMFReader::NUMBER:
						properties.setNumber(name,message.readNumber());
						break;
					case AMFReader::STRING: {
						string value;
						properties.setString(name, message.readString(value));
						break;
					}
					case AMFReader::DATE: {
						Date date;
						properties.setNumber<Int64>(name,message.readDate(date));
						break;
					}
					default:
						message.next();
				}
			}

			if (peer.path.empty() && properties.getString("tcUrl",_buffer)) {
				Util::UnpackUrl(_buffer, (string&)peer.serverAddress,(string&)peer.path,(string&)peer.query);
				Util::UnpackQuery(peer.query, properties);
			}

			if (properties.getNumber<UInt32,3>("objectEncoding")==0) {
				writer.amf0 = true;
				WARN("Client ",peer.protocol," not compatible with AMF3, few complex object can be not supported");
			}
		}
		if(external)
			ERROR("External type not acceptable for a connection message on flash stream ", id);

		message.startReferencing();

		
		// Check if the client is authorized
		AMFWriter& response = writer.writeAMFSuccess("NetConnection.Connect.Success","Connection succeeded",true);
		response.amf0 = true;
		response.writeNumberProperty("objectEncoding",writer.amf0 ? 0.0 : 3.0);
		response.amf0 = writer.amf0;
		Exception ex;
		peer.onConnection(ex, writer,message,response);
		if (ex) {
			switch(ex.code()) {
				case Exception::SOFTWARE:
					writer.writeAMFError("NetConnection.Connect.Rejected",ex.error());
					break;
				case Exception::APPLICATION:
					writer.writeAMFError("NetConnection.Connect.InvalidApp",ex.error());
					break;
				default:
					writer.writeAMFError("NetConnection.Connect.Failed",ex.error());
			}
			writer.writeInvocation("close");
			writer.close();
			return;
		}
			
		response.endObject();

	} else if(name == "setPeerInfo") {

		peer.localAddresses.clear();
		while(message.available()) {
			SocketAddress address;
			Exception ex;
			_buffer.clear();
			if (message.readString(_buffer).empty())
				continue;
			if (address.set(ex, _buffer))
				peer.localAddresses.emplace(address);
			if (ex)
				WARN("Peer address, ", ex.error())
		}
		
		BinaryWriter& response = writer.writeRaw();
		response.write16(0x29); // Unknown!
		response.write32(peer.parameters().getNumber<UInt32>("keepaliveServer")*1000);
		response.write32(peer.parameters().getNumber<UInt32>("keepalivePeer")*1000);

	} else if(name == "createStream") {
		shared_ptr<FlashStream>& pStream = invoker.createFlashStream(peer);
		_streams.emplace(pStream->id, pStream);
		AMFWriter& response(writer.writeMessage());
		response.amf0 = true;
		response.writeNumber(pStream->id);

	} else if(name == "deleteStream") {
		UInt32 id = (UInt32)message.readNumber();
		_streams.erase(id);
		invoker.destroyFlashStream(id);
	} else {
		// not close the main flash stream for that!
		Exception ex;
		if (!peer.onMessage(ex, name, message))
			ERROR(ex.set(Exception::APPLICATION, "Method '", name, "' not found on application ", peer.path).error())
		if (ex)
			writer.writeAMFError("NetConnection.Call.Failed", ex.error());
	}
}

void FlashMainStream::rawHandler(UInt8 type,PacketReader& packet,FlashWriter& writer) {

	// join group
	if(type==0x01) {
		if(packet.available()>0) {
			UInt32 size = packet.read7BitValue()-1;
			if(packet.read8()==0x10) {
				if (size > packet.available()) {
					ERROR("Bad header size for RTMFP group id")
					size = packet.available();
				}
				UInt8 groupId[ID_SIZE];
				EVP_Digest(packet.current(),size,(unsigned char *)groupId,NULL,EVP_sha256(),NULL);
				_pGroup = &peer.joinGroup(groupId, &writer);
			} else
				_pGroup = &peer.joinGroup(packet.current(), &writer);
		}
		return;
	}
	

	UInt16 flag = packet.read16();

	// setBufferTime
	if(flag==0x03) {
		UInt32 streamId = packet.read32();
		if(streamId==0) {
			bufferTime(packet.read32());
			return;
		}
		FlashStream* pStream = stream(streamId);
		if (!pStream) {
			ERROR("setBufferTime message for a unknown ",streamId," stream")
			return;
		}
		// To do working the buffertime on receiver side
		BinaryWriter& raw = writer.writeRaw();
		raw.write16(0);
		raw.write32(pStream->id);
		pStream->bufferTime(packet.read32());
		return;
	}

	
	 // ping message
	if(flag==0x06) {
		writer.writePong(packet.read32());
		return;
	}

	 // pong message
	if(flag==0x07) {
		peer.pong();
		return;
	}

	ERROR("Raw message ",Format<UInt8>("%.2x",type),"/",flag," unknown on stream ",id);	
}



} // namespace Mona
