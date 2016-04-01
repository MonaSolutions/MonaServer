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
#include "Mona/File.h"

namespace Mona {

// Error id

class Handler : public Invoker, public virtual Object {
public:

	//events	
	virtual	void			onRendezVousUnknown(const std::string& protocol,const UInt8* id,std::set<SocketAddress>& addresses){}
	virtual void			onHandshake(const std::string& protocol,const SocketAddress& address,const std::string& path,const Parameters& properties,UInt32 attempts,std::set<SocketAddress>& addresses){}
	virtual void			onConnection(Exception& ex,Client& client,DataReader& parameters,DataWriter& response){} // Exception::SOFTWARE, Exception::APPLICATION
	virtual void			onDisconnection(const Client& client){}
	virtual void			onAddressChanged(const Client& client,const SocketAddress& oldAddress) {}
	virtual bool			onMessage(Exception& ex, Client& client, const std::string& name, DataReader& reader, UInt8 responseType) { return false; } // Exception::SOFTWARE, Exception::APPLICATION
	virtual bool			onFileAccess(Exception& ex, Client& client,Client::FileAccessType type, DataReader& parameters, File& file, DataWriter& properties){return true;}  // Exception::SOFTWARE

	virtual void			onJoinGroup(Client& client,Group& group){}
	virtual void			onUnjoinGroup(Client& client,Group& group){}

	virtual bool			onPublish(Exception& ex,const Publication& publication, Client* pClient){return true;}
	virtual void			onUnpublish(const Publication& publication, Client* pClient){}

	virtual bool			onSubscribe(Exception& ex, Client& client,const Listener& listener){return true;}
	virtual void			onUnsubscribe(Client& client,const Listener& listener){}

protected:
	Handler(UInt32 socketBufferSize, UInt16 threads) : Invoker(socketBufferSize, threads) {}

private:
	
	virtual bool			onPublish(Exception& ex,const Publication& publication){return onPublish(ex,publication,NULL);}
	virtual void			onUnpublish(const Publication& publication) { onUnpublish(publication, NULL); }

};



} // namespace Mona
