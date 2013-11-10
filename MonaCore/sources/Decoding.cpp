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

#include "Mona/Decoding.h"
#include "Mona/Session.h"


using namespace std;

namespace Mona {

Decoding::Decoding(const shared_ptr<Buffer<UInt8>> &pBuffer, TaskHandler& taskHandler, UInt32 offset) :
	Task(taskHandler), _pBuffer(pBuffer), _reader(pBuffer->data(), pBuffer->size()) {
	if (offset)
		_reader.next(offset);
}

bool Decoding::run(Exception& exc) {
	Exception ex;
	bool success = false;
	EXCEPTION_TO_LOG(success=decode(ex, _reader), "Decoding")
	if (success)
		waitHandle();
	return true;
}

void Decoding::handle(Exception& ex) {
	unique_lock<mutex> lock;
	Session* pSession = _expirableSession.safeThis(lock);
	if (!pSession)
		return;
	if (_address.host().isWildcard())
		pSession->receive(_reader);
	else
		pSession->receive(_reader, _address);
}


} // namespace Mona
