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
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>

namespace Mona {

template<typename Result,typename... ArgsType>
class Event {
private:

	struct Function {
		friend class Event;
	public:
		Function() : _subscriptions(0) {}
		template<typename F> 
		Function(F f) : _subscriptions(0),_function(f) {}
		Function(const Function& other) : _subscriptions(0), _function(other._function) {}
		~Function() { if (_subscriptions>0) FATAL_ERROR("Deleting ", typeid(_function).name()," during event subscription"); }
		template< typename F >
		Function& operator=(F&& f) { if (_subscriptions>0) FATAL_ERROR("Changing ", typeid(_function).name()," during event subscription"); _function.operator=(f); return *this; }
		template< typename... Args >
		Result operator()(Args&&... args) const { return _function(args ...); }
		operator bool() const { return _function.operator bool(); }
	private:
		std::function<Result(ArgsType...)>	_function;
		mutable std::atomic<UInt32>			_subscriptions;
	};


public:
	typedef Function Type;

	bool subscribed() const {
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		return _pFunction || _pRelayer;
	}

	void subscribe(const Type& function) const {
		if (!function)
			return; // no change during listening, so if function is empty => useless listening
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		if (_pFunction == &function)
			return;
		if (_pFunction || _pRelayer)
			FATAL_ERROR("Event ", typeid(*this).name()," subscription has already a subscriber");
		++function._subscriptions;
		_pFunction = &function;
	}
	void subscribe(Event<Result(ArgsType...)>& event) const {
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		if (_pRelayer == &event)
			return;
		if (_pFunction || _pRelayer)
			FATAL_ERROR("Event ", typeid(*this).name()," subscription has already a subscriber");
		++event._relay;
		_pRelayer = &event;
	}
	void unsubscribe(const Type& function) const {
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		if (&function != _pFunction)
			return;
		--function._subscriptions;
		_pFunction = NULL;
	}
	void unsubscribe(Event<Result(ArgsType...)>& event) const {
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		if (&event != _pRelayer)
			return;
		--_pRelayer->_relay;
		_pRelayer = NULL;
	}

protected:
	Event() : _pFunction(NULL),_relay(0),_pRelayer(NULL),_pMutex(new std::recursive_mutex()) {}
	virtual ~Event() {
		// no need of mutex here, because no more method can be called on an deleting object
		if (_relay>0)
			FATAL_ERROR("Deleting function during event ", typeid(*this).name()," subscription");
		if (_pFunction)
			--_pFunction->_subscriptions;
		if (_pRelayer)
			--_pRelayer->_relay;
	}

	/*
	void lock(std::unique_lock<std::recursive_mutex>& guard) {
		guard = std::unique_lock<std::recursive_mutex>(*_pMutex);
	}*/


private:
	std::shared_ptr<std::recursive_mutex>	_pMutex;
	mutable const Type*						_pFunction;
	mutable Event<Result(ArgsType...)>*		_pRelayer;
	mutable std::atomic<UInt32>				_relay;

	friend class Event<void(ArgsType ...)>;
	friend class Event<Result(ArgsType ...)>;
};


template<typename... ArgsType>
class Event<void(ArgsType ...)> : public Event<void,ArgsType ...>, public virtual Object  {
public:
	template <typename ...Args>
	void raise(Args&&... args) {
		std::shared_ptr<std::recursive_mutex> pMutex(Event<void,ArgsType ...>::_pMutex); // because the event can delete this!
		std::lock_guard<std::recursive_mutex> lock(*pMutex);
		if (Event<void,ArgsType ...>::_pRelayer)
			Event<void,ArgsType ...>::_pRelayer->raise(args...);
		else if (Event<void,ArgsType ...>::_pFunction)
			(*Event<void,ArgsType ...>::_pFunction)(args ...);
	}
};


template<typename Result,typename... ArgsType>
class Event<Result(ArgsType ...)>: public Event<Result,ArgsType ...>, public virtual Object {
public:
	template <Result defaultResult,typename ...Args>
	Result raise(Args&&... args) {
		std::shared_ptr<std::recursive_mutex> pMutex(Event<Result,ArgsType ...>::_pMutex); // because the event can delete this!
		std::lock_guard<std::recursive_mutex> lock(*pMutex);
		if (Event<Result,ArgsType ...>::_pRelayer)
			return Event<Result,ArgsType ...>::_pRelayer->template raise<defaultResult>(args...);
		else if (Event<Result, ArgsType ...>::_pFunction)
			return (*Event<Result, ArgsType ...>::_pFunction)(args ...);
		return defaultResult;
	}

};

} // namespace Mona
