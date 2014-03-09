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
#include "Mona/DataReader.h"
#include "Mona/QualityOfService.h"
#include "Mona/PacketReader.h"
#include <set>

namespace Mona {


class Writer;
class WriterHandler : virtual Object {	
public:
	virtual void	close(Writer& writer,int code){}
};

class Client;
class Writer : virtual NullableObject {
public:
	enum MediaType {
		INIT=0,
		AUDIO,
		VIDEO,
		DATA,
		START,
		STOP
	};
	enum State {
		GET,
		CONNECTING,
		CONNECTED,
		CLOSED
	};

	static void DumpResponse(const UInt8* data, UInt32 size, const SocketAddress& address, bool justInDebug = false);

	bool					reliable;


	const QualityOfService&	qos() { return _qos; }


	virtual Writer&			newWriter(WriterHandler& handler) { _handlers.insert(&handler); return *this; }

	virtual State			state(State value=GET,bool minimal=false);


	
	/**	The main Writer of one session should close the entiere session
		If code==0, it's a normal close
		If code>0, it's a user close (from server application script)
		If code<0, it's a system core close
			-1 => Listener close!				*/
	virtual void			close(int code=0);

	
	/**	Call  by Multimedia framework on a writer Listener, on subscription. The first call takes a type=INIT parameter, 
		with a time=0, and with a packet argument which contains the publication name.
		If this call returns false, the writer can be closed, and subscription have to be removed by the writer handler.
		If it returns true, for every media type (AUDIO, VIDEO, DATA), an type=INIT call is invoked on start with time=MediaType,
		and always publication name in packet.
		Finally, every media data are passed (AUDIO, VIDEO and DATA), if the methods returns false, the cycle restart since the beginning */
	virtual bool			writeMedia(MediaType type,UInt32 time,PacketReader& packet);
	virtual bool			writeMember(const Client& client);

    virtual DataWriter&		writeInvocation(const std::string& name){return DataWriter::Null;}
    virtual DataWriter&		writeMessage(){return DataWriter::Null;}
	virtual DataWriter&		writeResponse(UInt8 type){return writeMessage();}
	virtual void			writeRaw(const UInt8* data,UInt32 size){}

	virtual void			flush(bool full=false){}

	virtual void			createReader(PacketReader& packet,std::shared_ptr<DataReader>& pReader) {}
	virtual void			createWriter(std::shared_ptr<DataWriter>& pWriter) {}
	virtual bool			hasToConvert(DataReader& reader) {return false;}

    static Writer			Null;

protected:
	Writer(WriterHandler* pHandler=NULL);
	Writer(Writer& writer);
	Writer(bool isNull);
	virtual ~Writer();

	QualityOfService				_qos;
	
private:
	std::set<WriterHandler*>		_handlers;
	State							_state;
};


} // namespace Mona
