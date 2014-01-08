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
#include "Mona/Exceptions.h"
#include "Mona/Invoker.h"
#include "Mona/FilePath.h"

namespace Mona {

// Error id

class Handler : public Invoker, virtual Object {
public:
	//events	
	virtual	void			onRendezVousUnknown(const std::string& protocol,const UInt8* id,std::set<SocketAddress>& addresses){}
	virtual void			onHandshake(const std::string& protocol,const SocketAddress& address,const std::string& path,const MapParameters& properties,UInt32 attempts,std::set<SocketAddress>& addresses){}
	virtual void			onConnection(Exception& ex,Client& client,DataReader& parameters,DataWriter& response){} // Exception::SOFTWARE, Exception::APPLICATION
	virtual void			onDisconnection(const Client& client){}
	virtual void			onMessage(Exception& ex, Client& client,const std::string& name,DataReader& reader,UInt8 responseType){} // Exception::SOFTWARE, Exception::APPLICATION
	virtual bool			onRead(Exception& ex, Client& client,FilePath& filePath,DataReader& parameters,DataWriter& properties){return true;}  // Exception::SOFTWARE

	virtual void			onJoinGroup(Client& client,Group& group){}
	virtual void			onUnjoinGroup(Client& client,Group& group){}

	virtual bool			onPublish(Client& client,const Publication& publication,std::string& error){return true;}
	virtual void			onUnpublish(Client& client,const Publication& publication){}

	virtual void			onDataPacket(Client& client,const Publication& publication,DataReader& packet){}
	virtual void			onAudioPacket(Client& client,const Publication& publication,UInt32 time,PacketReader& packet){}
	virtual void			onVideoPacket(Client& client,const Publication& publication,UInt32 time,PacketReader& packet){}
	virtual void			onFlushPackets(Client& client,const Publication& publication){}

	virtual bool			onSubscribe(Client& client,const Listener& listener,std::string& error){return true;}
	virtual void			onUnsubscribe(Client& client,const Listener& listener){}

protected:
	Handler(UInt32 bufferSize, UInt16 threads) : _myself(*this), Invoker(bufferSize, threads) {
		Util::Random(id, ID_SIZE); // Allow to publish in intern (Invoker is the publisher)
		(bool&)_myself.connected=true;
		std::memcpy((UInt8*)myself().id,id,ID_SIZE);
	}
	virtual ~Handler(){}
private:
	Peer&					myself() { return _myself; }
	Peer					_myself;
};



} // namespace Mona
