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
#include "Mona/TCPSession.h"
#include "Mona/WebSocket/WSWriter.h"
#include "Mona/RawReader.h"


namespace Mona {


class WSSession : public TCPSession, public virtual Object {
public:

	WSSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker);

	bool			buildPacket(PoolBuffer& pBuffer,PacketReader& packet);
	void			packetHandler(PacketReader& packet);
	void			flush() { if (_pPublication) _pPublication->flush(); Session::flush(); }
	void			manage();

	/// \brief Read message and call method if needed
	/// \param packet Content message to read
	template<typename ReaderType>
	void readMessage(Exception& ex, PacketReader& packet) {

		ReaderType reader(packet);
		if(!reader.isValid()) {
			packet.reset();
			RawReader rawReader(packet);
			if (!peer.onMessage(ex, "onMessage", rawReader, WS::TYPE_TEXT))
				ex.set(Exception::APPLICATION, "Method 'onMessage' not found on application ", peer.path);
			return;
		}

		if (reader.followingType()==DataReader::STRING) {

			std::string name;
			reader.readString(name);
			if(name=="__publish") {
				if(reader.followingType()!=DataReader::STRING) {
					ex.set(Exception::PROTOCOL, "__publish method takes a stream name in first parameter",WS::CODE_MALFORMED_PAYLOAD);
					return;
				}
				reader.readString(name);
				if(_pPublication)
					invoker.unpublish(peer,_pPublication->name());
				Publication::Type type(Publication::LIVE);
				if (reader.followingType() == DataReader::STRING) {
					std::string temp;
					if(reader.readString(temp).compare("record") == 0)
						 type = Publication::RECORD;
				}
					
				_pPublication = invoker.publish(ex, peer,name,type);
			} else if(name=="__play") {
				if(reader.followingType()!=DataReader::STRING) {
					ex.set(Exception::PROTOCOL, "__play method takes a stream name in first parameter",WS::CODE_MALFORMED_PAYLOAD);
					return;
				}
				reader.readString(name);
					
				closeSusbcription();
			} else if(name=="__closePublish")
				closePublication();
			else if(name=="__closePlay")
				closeSusbcription();
			else if (name == "__close") {
				closePublication();
				closeSusbcription();
			} else if(_pPublication) {
				reader.reset();
				_pPublication->pushData(reader);
			} else if (!peer.onMessage(ex, name,reader))
				ex.set(Exception::APPLICATION, "Method '",name,"' not found on application ", peer.path);
		} else {
			reader.reset();
			if(!peer.onMessage(ex, "onMessage",reader))
				ex.set(Exception::APPLICATION, "Method 'onMessage' not found on application ", peer.path);
		}
	}

protected:
	WSWriter&		wsWriter() { return _writer; }
	void			kill(UInt32 type=NORMAL_DEATH);
	Publication*	_pPublication;
	Listener*		_pListener;
	
private:
	void			closeSusbcription();
	void			closePublication();

	WSWriter		_writer;
	Time			_time;
};


} // namespace Mona
