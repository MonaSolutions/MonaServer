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

#include "Mona/Mona.h"
#include "Mona/Task.h"
#include "Mona/WorkThread.h"
#include "Mona/Protocol.h"


namespace Mona {


class Session;
class Decoding : public WorkThread, private Task, virtual Object {
public:
	Decoding(UInt32 id, TaskHandler& taskHandler, Protocol& protocol, const std::shared_ptr<MemoryReader>& pReader, const SocketAddress& address);
	virtual ~Decoding();

	const UInt32		 id;
	const SocketAddress	 address;

	MemoryReader&		reader() { return *_pReader; };
	Session*			session() { return _protocol.session(id, *_pReader); }

protected:
	Decoding(UInt32 id,TaskHandler& taskHandler,Protocol& protocol,const std::shared_ptr<Buffer<UInt8>>& pBuffer,const SocketAddress& address);

private:
	// If ex is raised, an error is displayed if the operation has returned false
	// otherwise a warning is displayed
	virtual bool				decode(Exception& ex,MemoryReader& reader){return false;}

	void						handle(Exception& ex) { _protocol.receive(*this); }
	bool						run(Exception& ex);

	std::shared_ptr<MemoryReader>	_pReader;
	std::shared_ptr<Buffer<UInt8> >	_pBuffer;
	Protocol&						_protocol;
};



} // namespace Mona
