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

#pragma once


#include "Mona/SocketManager.h"
#include "Mona/ServerSocket.h"


namespace Mona {

class TCPServer : protected ServerSocket, virtual Object {
public:
	TCPServer(const SocketManager& manager);
	virtual ~TCPServer();

	bool		start(Exception& ex, UInt16 port);
	bool		running() { return _port > 0; }
	UInt16		port() { return _port; }
	void		stop();

	template <typename ClientType,typename ...Args>
	ClientType* acceptClient(Exception& ex, Args&... args) {
		ASSERT_RETURN(_hasToAccept == true, NULL)
		ClientType* pClient = acceptConnection<ClientType>(ex, args ...);
		_hasToAccept = false;
		return pClient;
	}

private:
	enum ReceptionState {
		NOTHING,
		REQUESTING
	};

	virtual void	onClientRequest(Exception& ex) = 0;
	virtual void	onReadable(Exception& ex);

	UInt16			_port;
	bool		  _hasToAccept;
};


} // namespace Mona
