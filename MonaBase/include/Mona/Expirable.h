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
#include <memory>
#include <mutex>
#include <atomic>


namespace Mona {

template<typename ObjectType>
class Expirable : virtual Object {
public:
	Expirable() : _pOwner(NULL), _isOwner(false) {}

	virtual ~Expirable() {
		if (_isOwner)
			FATAL_ASSERT(*_pExpired);
	}

	bool isOwner() const { return _isOwner; }

	ObjectType* safeThis(std::unique_lock<std::mutex>& guard) {
		if (!_pOwner)
			return NULL;
		if (!_isOwner)
			guard = std::unique_lock<std::mutex>(*_pMutex);
		return *_pExpired ? NULL : _pOwner;
	}

	ObjectType* unsafeThis() {
		if (!_pOwner)
			return NULL;
		return *_pExpired ? NULL : _pOwner;
	}

	void shareThis(Expirable& other) {
		FATAL_ASSERT(other._isOwner == false)
		other._pOwner = _pOwner;
		other._pMutex = _pMutex;
		other._pExpired = _pExpired;
	}

	void expire() {
		if (!_isOwner || *_pExpired)
			return;
		std::lock_guard<std::mutex> lock(*_pMutex);
		*_pExpired = true;
	}

	
protected:
	Expirable(ObjectType* pThis) : _isOwner(true), _pMutex(new std::mutex()), _pOwner(pThis), _pExpired(new std::atomic<bool>(false)) {}

	
private:
	std::shared_ptr<std::mutex>			_pMutex;
	std::shared_ptr<std::atomic<bool>>	_pExpired;
	ObjectType*							_pOwner;
	bool								_isOwner;
};

} // namespace Mona
