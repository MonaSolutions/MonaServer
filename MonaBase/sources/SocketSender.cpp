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

#include "Mona/SocketSender.h"
#include "Mona/SocketManager.h"
#include "Mona/Logs.h"
#include "Poco/Format.h"

using namespace std;
using namespace Poco;

namespace Mona {


SocketSender::SocketSender(SocketHandlerBase& handler,const UInt8* data,UInt32 size,bool dump) : _wrote(false),_position(0),_data((UInt8*)data),_size(size),_memcopied(false),_dump(dump),_handler(handler),_running(false) {
}

SocketSender::SocketSender(SocketHandlerBase& handler,bool dump) : _wrote(false),_position(0),_data(NULL),_size(0),_memcopied(false),_dump(dump),_handler(handler),_running(false)  {
}


SocketSender::~SocketSender() {
	if(_memcopied)
		delete [] _data;
}

PoolThread* SocketSender::go(Exception& ex, PoolThread* pThread) {
	_running=true;
	_pSocketClosed = _handler._pClosed;
	duplicate();

	PoolThread* plthread = _handler.manager.poolThreads.enqueue(ex, this,pThread);
	if (ex)
		return NULL;

	return plthread;
}

void SocketSender::release() {
	if(!_running && !_wrote && referenceCount()==1 && available()) {
		_wrote=true;
		_handler.writeSocket(this);
		return;
	}
	WorkThread::release();
}

void SocketSender::run() {
	if(!_running)
		return;
	if(!_pSocketClosed.isNull() && !*_pSocketClosed) {
		duplicate();
		_handler.writeSocket(this);
	}
}

void SocketSender::dump(bool justInDebug) {
	if(!_dump)
		return;
	if(!justInDebug || justInDebug&&Logs::GetLevel()>=7)
		DUMP(begin(true),size(true),Poco::format("Response to %s",receiver().toString()).c_str())
	_dump=false;
}

bool SocketSender::flush() {
	if(!available())
		return true;
	dump();
	try {
		_position += send(begin()+_position,size()-_position);
		if(_position == size())
			return true;
		if(!_memcopied && _data==begin()) {
			_size = _size-_position;
			UInt8* temp = new UInt8[_size]();
			memcpy(temp,_data+_position,_size);
			_data = temp;
			_position = 0;
			_memcopied = true;
		}
		return false;
	} catch(Poco::Exception& ex) {
		 WARN("Socket sending error, ",ex.displayText());
	} catch(exception& ex) {
		 WARN("Socket sending error, ",ex.what());
	} catch(...) {
		 WARN("Socket sending unknown error");
	}
	_position = size();
	return true;
}


} // namespace Mona
