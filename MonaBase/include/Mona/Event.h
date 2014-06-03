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
#include <set>

namespace Mona {

template<typename Result,typename... ArgsType>
class Event {
private:

	struct Function {
		friend class Event;

	public:
		Function() : _pTrigger(NULL) {}
		template<typename F> 
		Function(F f) : _function(f),_pTrigger(NULL) {}
		Function(const Function& other) : _function(other._function),_pTrigger(NULL) {}
		~Function() { if (!_events.empty()) FATAL_ERROR("Deleting ", typeid(_function).name()," during ",_events.size()," event subscription"); }
		template< typename F >
		Function& operator=(F&& f) { if (!_events.empty()) FATAL_ERROR("Changing ", typeid(_function).name()," during ",_events.size()," event subscription"); _function.operator=(f); return *this; }
		template< typename... Args >
		Result operator()(Args&&... args) const { return _function(args ...); }
		operator bool() const { return _function.operator bool(); }

		template<typename TriggerType>
		TriggerType* trigger() { return _pTrigger; }

		void unsubscribe() const { while(!_events.empty()) (*_events.begin())->unsubscribe(*this); }
	private:
		std::function<Result(ArgsType...)>	_function;
		mutable std::set<const Event*>		_events;
		mutable Event*						_pTrigger;

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
		if ((_pFunction && _pFunction!=&function) || _pRelayer)
			FATAL_ERROR("Event ", typeid(*this).name()," subscription has already a subscriber");
		function._events.emplace(this);
		_pFunction = &function;
	}
	void subscribe(Event<Result(ArgsType...)>& event) const {
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		if (_pFunction || (_pRelayer && _pRelayer != &event))
			FATAL_ERROR("Event ", typeid(*this).name()," subscription has already a subscriber");
		event._relayed = true;
		_pRelayer = &event;
	}
	void unsubscribe(const Type& function) const {
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		if (_pRelayer || (_pFunction && _pFunction != &function))
			FATAL_ERROR("Bad ", typeid(*this).name()," unsubscription function");
		function._events.erase(this);
		_pFunction = NULL;
	}
	void unsubscribe(Event<Result(ArgsType...)>& event) const {
		std::lock_guard<std::recursive_mutex> lock(*_pMutex);
		if (_pFunction || (_pRelayer && _pRelayer != &event))
			FATAL_ERROR("Bad ", typeid(*this).name()," unsubscription event");
		event._relayed = false;
		_pRelayer = NULL;
	}
	
protected:
	Event() : _pFunction(NULL),_relayed(false),_pRelayer(NULL),_pMutex(new std::recursive_mutex()) {}
	virtual ~Event() {
		if (_relayed)
			FATAL_ERROR("Deleting function during event ", typeid(*this).name()," subscription");
		if (_pFunction)
			_pFunction->_events.erase(this);
		if (_pRelayer)
			_pRelayer->_relayed = false;
	}

	void lock(std::unique_lock<std::recursive_mutex>& guard) {
		guard = std::unique_lock<std::recursive_mutex>(*_pMutex);
	}


private:
	void setTrigger(Event* pTrigger) { if(_pFunction) _pFunction->_pTrigger = pTrigger; }

	std::shared_ptr<std::recursive_mutex>	_pMutex;
	mutable const Type*						_pFunction;
	mutable Event<Result(ArgsType...)>*		_pRelayer;
	bool									_relayed;

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
		else if (Event<void, ArgsType ...>::_pFunction) {
			Event<void, ArgsType ...>::setTrigger(this);
			(*Event<void, ArgsType ...>::_pFunction)(args ...);
			Event<void, ArgsType ...>::setTrigger(NULL);
		}
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
			return Event<Result,ArgsType ...>::_pRelayer->raise<defaultResult>(args...);
		else if (Event<Result, ArgsType ...>::_pFunction) {
			Event<Result, ArgsType ...>::setTrigger(this);
			Result result((*Event<Result, ArgsType ...>::_pFunction)(args ...));
			Event<Result, ArgsType ...>::setTrigger(NULL);
			return result;
		}
		return defaultResult;
	}

};

} // namespace Mona
