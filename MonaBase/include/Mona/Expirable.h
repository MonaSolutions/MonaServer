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
#include <memory>
#include <mutex>



namespace Mona {

template<typename ObjectType>
class Expirable : virtual Object {
public:
	Expirable() : _pOwner(NULL), _isOwner(false) {}


	virtual ~Expirable() {
		if (_isOwner)
			FATAL_ASSERT(*_pExpired);
	}

	ObjectType* safeThis(std::unique_lock<std::mutex>& guard) {
		if (!_pOwner)
			return NULL;
		if (!_isOwner)
			guard = std::unique_lock<std::mutex>(*_pMutex);
		return *_pExpired ? NULL : _pOwner;
	}

	bool shareThis(Exception& ex, Expirable& other) {
		ASSERT_RETURN(other._isOwner==false,false);
		other._pOwner = _pOwner;
		other._pMutex = _pMutex;
		other._pExpired = _pExpired;
		return true;
	}
	
protected:
	Expirable(ObjectType* pThis) : _isOwner(true), _pMutex(new std::mutex()), _pOwner(pThis), _pExpired(new bool(false)) {}

	void expire() {
		if (!_isOwner)
			return;
		std::lock_guard<std::mutex> lock(*_pMutex);
		*_pExpired = true;
	}

private:
	std::shared_ptr<std::mutex>	_pMutex;
	std::shared_ptr<bool>		_pExpired;
	ObjectType*					_pOwner;
	bool						_isOwner;
};

} // namespace Mona
