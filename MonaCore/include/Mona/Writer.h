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
#include "Mona/DataReader.h"
#include "Mona/QualityOfService.h"
#include "Mona/MemoryReader.h"
#include "Mona/Logs.h"

namespace Mona {

#define FLUSH_SENDERS(ERROR_HEADER,SENDERTYPE,SENDERS) { Exception __ex; for (std::shared_ptr<SENDERTYPE>& pSender : SENDERS) { _socket.send<SENDERTYPE>(__ex, pSender); if (__ex) ERROR(ERROR_HEADER,", ",__ex.error()); } SENDERS.clear();}

class Writer;
class WriterHandler : virtual Object {	
public:
	virtual void	close(Writer& writer,int code){}
};

class Peer;
class WriterNull;

class Writer : virtual ObjectNullable {
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

	bool					reliable;

	const QualityOfService&	qos() { return _qos; }


	virtual Writer&			newWriter(WriterHandler* pHandler = NULL) { return *this; }

	virtual State			state(State value=GET,bool minimal=false);
	virtual void			close(int code=0);

	virtual bool			writeMedia(MediaType type,UInt32 time,MemoryReader& data);
	virtual void			writeMember(const Peer& peer);

    virtual DataWriter&		writeInvocation(const std::string& name){return DataWriter::Null;}
    virtual DataWriter&		writeMessage(){return DataWriter::Null;}
	virtual void			writeRaw(const UInt8* data,UInt32 size){}

	virtual void			flush(bool full=false){}

	virtual void			createReader(MemoryReader& reader,std::shared_ptr<DataReader>& pReader) {}
	virtual void			createWriter(std::shared_ptr<DataWriter>& pWriter) {}
	virtual bool			hasToConvert(DataReader& reader) {return false;}

    static WriterNull		Null;

protected:
	Writer(WriterHandler* pHandler=NULL);
	Writer(Writer& writer);
	virtual ~Writer();

	QualityOfService		_qos;
	WriterHandler*			_pHandler;
	State					_state;
};

class WriterNull : public Writer, virtual ObjectNullable {
public:
    WriterNull() : ObjectNullable(true), Writer(NULL) {}
};


} // namespace Mona
