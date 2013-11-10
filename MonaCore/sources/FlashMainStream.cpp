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

#include "Mona/FlashMainStream.h"
#include "Mona/Invoker.h"
#include "Mona/Util.h"
#include "Mona/Exceptions.h"
#include "Mona/Logs.h"
#include "Mona/Buffer.h"
#include <openssl/evp.h>


using namespace std;


namespace Mona {


FlashMainStream::FlashMainStream(Invoker& invoker,Peer& peer) : FlashStream(0,invoker,peer),_pGroup(NULL) {
	
}

FlashMainStream::~FlashMainStream() {
	if(_pGroup)
		peer.unjoinGroup(*_pGroup);
	// delete stream index remaining (which have not had time to send a 'destroyStream' message)
	set<UInt32>::const_iterator it;
	for(it=_streams.begin();it!=_streams.end();++it)
		invoker.destroyFlashStream(*it);
}

void FlashMainStream::close(FlashWriter& writer,const string& error,int code) {
	switch(code) {
		case Exception::SOFTWARE:
			writer.writeAMFError("NetConnection.Connect.Rejected",error);
			break;
		case Exception::APPLICATION:
			writer.writeAMFError("NetConnection.Connect.InvalidApp",error);
			break;
		default:
			writer.writeAMFError("NetConnection.Connect.Failed",error);
	}
	if(!error.empty())
		ERROR(error)
	writer.writeInvocation("close");
	writer.close();
}


void FlashMainStream::messageHandler(Exception& ex, const string& name,AMFReader& message,FlashWriter& writer) {
	if(name=="connect") {
		message.stopReferencing();
		AMFReader::Type type;
		string name;
		bool external=false;
		if(message.readObject(name,external)) {
			if(external) {
				ex.set(Exception::PROTOCOL, "External type not acceptable for a connection message");
				return;
			}
			while((type=message.readItem(name))!=AMFReader::END) {
				switch(type) {
					case AMFReader::NIL:
						message.readNull();
						break;
					case AMFReader::BOOLEAN:
						peer.setBool(name,message.readBoolean());
						break;
					case AMFReader::NUMBER:
						peer.setNumber(name,message.readNumber());
						break;
					case AMFReader::STRING: {
						string value;
						peer.setString(name, message.readString(value));
						break;
					}
					case AMFReader::TIME: {
						Time time;
						peer.setNumber(name, (double)(message.readTime(time) / 1000));
						break;
					}
					default:
						message.next();
						return;
				}
			}
		}
		message.startReferencing();

		string tcUrl;
		Exception exWarn;
		if (peer.path.empty() && peer.getString("tcUrl", tcUrl))
			Util::UnpackUrl(exWarn, tcUrl, (SocketAddress&)peer.serverAddress, (string&)peer.path, peer);
		if (exWarn)
			WARN("serverAddress impossible to determine from url ", tcUrl);
		if (peer.serverAddress.port() == 0) {
			string protocol;
			if (peer.getString("protocol", protocol)) {
				if (protocol == "RTMP")
					((SocketAddress&)peer.serverAddress).set(peer.serverAddress.host(), invoker.params.RTMP.port);
				else if (protocol == "RTMFP")
					((SocketAddress&)peer.serverAddress).set(peer.serverAddress.host(), invoker.params.RTMFP.port);
				else
					WARN("Impossible to determine the serverAddress port from unknown ",protocol," protocol")
			}
		}

	
		// Don't support AMF0 forced on NetConnection object because AMFWriter writes in AMF3 format
		// But it's not a pb because NetConnection RTMFP works since flash player 10.0 only (which supports AMF3)
		double objEncoding = 1;
		if (peer.getNumber("objectEncoding", objEncoding) && objEncoding==0) {
			ex.set(Exception::PROTOCOL, "ObjectEncoding client must be in a AMF3 format (not AMF0)");
			return;
		}

		// Check if the client is authorized
		AMFWriter& response = writer.writeAMFSuccess("NetConnection.Connect.Success","Connection succeeded",true);
		response.amf0Preference = true;
		response.writeNumberProperty("objectEncoding",3.0);
		response.amf0Preference = false;
		peer.onConnection(ex, writer,message,response);
		if (ex)
			return;
		response.endObject();


	} else if(name == "setPeerInfo") {

		peer.addresses.resize(1);
		string addr;
		while(message.available()) {
	
			peer.addresses.emplace_back();
			Exception ex;
			peer.addresses.back().set(ex, message.readString(addr));  // private host
			if (ex) {
				ERROR("Bad peer address ",addr,", ",ex.error());
				peer.addresses.pop_back();
			}
		}
		
		BinaryWriter& response = writer.writeRaw();
		response.write16(0x29); // Unknown!
		response.write32(invoker.params.RTMFP.keepAliveServer);
		response.write32(invoker.params.RTMFP.keepAlivePeer);

	} else if(name == "initStream") {
		// TODO?
	} else if(name == "createStream") {

		writer.writeMessage().writeNumber(*_streams.insert(invoker.createFlashStream(peer)).first);

	} else if(name == "deleteStream") {
		UInt32 id = (UInt32)message.readNumber();
		_streams.erase(id);
		invoker.destroyFlashStream(id);

	} else {
		Exception exm;
		peer.onMessage(exm, name, message);
		if (exm) {
			writer.writeAMFError("NetConnection.Call.Failed", exm.error());
		}		
	}
	
}

void FlashMainStream::rawHandler(Exception& ex,UInt8 type,MemoryReader& data,FlashWriter& writer) {

	if(type==0x01) {
		if(data.available()>0) {
			UInt32 size = data.read7BitValue()-1;
			UInt8 flag = data.read8();

			UInt8 groupId[ID_SIZE];

			if(flag==0x10) {
				Buffer<UInt8> groupIdVar(size);
				data.readRaw(&groupIdVar[0],size);
				EVP_Digest(&groupIdVar[0],groupIdVar.size(),(unsigned char *)groupId,NULL,EVP_sha256(),NULL);
			} else
				data.readRaw(groupId,ID_SIZE);
		
			_pGroup = invoker.groups(groupId);
		
			if(_pGroup)
				peer.joinGroup(*_pGroup,&writer);
			else
				_pGroup = &peer.joinGroup(groupId,&writer);
		}
	} else {
		UInt16 flag = data.read16();
		if(flag==0x03) {
			// setBufferTime
			UInt32 streamId = data.read32();
			if(streamId==0) {
				setBufferTime(data.read32());
				return;
			}
			shared_ptr<FlashStream> pStream = invoker.getFlashStream(streamId);
			if (!pStream) {
				ERROR("setBufferTime message for a unknown ",streamId," stream")
				return;
			}
			UInt32 ms = data.read32();
			INFO("setBufferTime ",ms," on stream ",pStream->id)
			// To do working the buffertime on receiver side
			BinaryWriter& raw = writer.writeRaw();
			raw.write16(0);
			raw.write32(pStream->id);
			pStream->setBufferTime(ms);
			return;
		}
		ERROR("Raw message ",Format<UInt8>("%.2x",type),"/",flag," unknown on stream ",id);
	}
		
}



} // namespace Mona
