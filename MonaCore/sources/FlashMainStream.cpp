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
#include "Mona/ParameterWriter.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

FlashMainStream::FlashMainStream(Invoker& invoker,Peer& peer) : FlashStream(0, invoker,peer) {
	
}

FlashMainStream::~FlashMainStream() {
	for (auto& it : _streams) {
		it.second->OnStart::unsubscribe((OnStart&)*this);
		it.second->OnStop::unsubscribe((OnStop&)*this);
	}
}

void FlashMainStream::disengage(FlashWriter* pWriter) {
	for (auto& it : _streams)
		it.second->disengage(pWriter);
}


FlashStream* FlashMainStream::getStream(UInt16 id,shared_ptr<FlashStream>& pStream) {
	const auto& it = _streams.find(id);
	if (it == _streams.end()) {
		pStream.reset();
		return NULL;
	}
	return (pStream = it->second).get();
}

void FlashMainStream::messageHandler(const string& name,AMFReader& message,FlashWriter& writer) {
	if(name=="connect") {
		message.stopReferencing();
		
		
		ParameterWriter parameterWriter(peer.properties());

		if(message.read(AMFReader::OBJECT,parameterWriter)) {

			if (peer.properties().getString("tcUrl",_buffer)) {
				string serverAddress;
				Util::UnpackUrl(_buffer, serverAddress,(string&)peer.path,(string&)peer.query);
				peer.setServerAddress(serverAddress);
				Util::UnpackQuery(peer.query, peer.properties());
			}

			if (peer.properties().getNumber<UInt32,3>("objectEncoding")==0) {
				writer.amf0 = true;
				WARN("Client ",peer.protocol," not compatible with AMF3, few complex object can be not supported");
			}
		}

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
			if (!message.readString(_buffer) || _buffer.empty())
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

		UInt16 idStream(1);
		auto it(_streams.begin());
		for (; it != _streams.end();++it) {
			if (it->first > idStream) 
				break;
			++idStream;
		}
		FlashStream* pStream(new FlashStream(idStream, invoker, peer));
		_streams.emplace_hint(it, piecewise_construct, forward_as_tuple(idStream), forward_as_tuple(pStream));

		pStream->OnStart::subscribe((OnStart&)*this);
		pStream->OnStop::subscribe((OnStop&)*this);

		AMFWriter& response(writer.writeMessage());
		response.amf0 = true;
		response.writeNumber(idStream);

	} else if(name == "deleteStream") {
		double streamId;
		if (message.readNumber(streamId)) {
			const auto& it = _streams.find((UInt16)streamId);
			if (it != _streams.end()) {
				it->second->OnStart::unsubscribe((OnStart&)*this);
				it->second->OnStop::unsubscribe((OnStop&)*this);
				_streams.erase(it);
			}
		}else
			ERROR("deleteStream message without id on flash stream ",streamId)
	} else {
		// not close the main flash stream for that!
		Exception ex;
		if (!peer.onMessage(ex, name, message)) {
			ex.set(Exception::APPLICATION, "Method client '", name, "' not found in application ", peer.path);
			ERROR(ex.error())
		}
		if (ex)
			writer.writeAMFError("NetConnection.Call.Failed", ex.error());
	}
}

void FlashMainStream::rawHandler(UInt16 type,PacketReader& packet,FlashWriter& writer) {

	// setBufferTime
	if(type==0x0003) {
		UInt16 streamId = packet.read32();
		if(streamId==0) {
			bufferTime(packet.read32());
			return;
		}
		const auto& it = _streams.find(streamId);
		if (it==_streams.end()) {
			ERROR("setBufferTime message for an unknown ",streamId," stream")
			return;
		}
		it->second->bufferTime(packet.read32());
		return;
	}

	
	 // ping message
	if(type==0x0006) {
		writer.writePong(packet.read32());
		return;
	}

	 // pong message
	if(type==0x0007) {
		peer.pong();
		return;
	}

	ERROR("Raw message ",Format<UInt16>("%.4x",type)," unknown on main stream");
}



} // namespace Mona
