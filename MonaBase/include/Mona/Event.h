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
#include <set>

namespace Mona {

template<typename... ArgsType>
class Event : virtual Object {
private:
	
	struct Function : std::function<void(ArgsType...)> {
		friend class Event;
	public:
		template< class... Args > 
		Function(Args... args) : _isListening(false), std::function<void(ArgsType...)>(args...) {}
		~Function() { if (_isListening) FATAL_ERROR("Deleting function is always listening its event"); }
	private:
		mutable bool _isListening;
	};


public:
	typedef Function Type;

	void addListener(const Type& function) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if(_listeners.emplace(&function).second)
			function._isListening = true;
	}
	void addListener(const Type& function,std::unique_lock<std::recursive_mutex>& guard) {
		guard = std::unique_lock<std::recursive_mutex>(_mutex);
		if(_listeners.emplace(&function).second)
			function._isListening = true;
	}
	void removeListener(const Type& function) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if(_listeners.erase(&function)>0)
			function._isListening = false;
	}
	void removeListener(const Type& function,std::unique_lock<std::recursive_mutex>& guard) {
		guard = std::unique_lock<std::recursive_mutex>(_mutex);
		if(_listeners.erase(&function)>0)
			function._isListening = false;
	}
	template <typename ...Args>
	void raise(Args&&... args) { 
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		for (const Type* pFunc : _listeners) {
			if (*pFunc)
				(*pFunc)(args ...);
		}
	}
private:
	std::recursive_mutex	_mutex;
	std::set<const Type*>	_listeners;
};


} // namespace Mona
