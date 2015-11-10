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

#include "Mona/Startable.h"
#include "Mona/Exceptions.h"
#include "Mona/PoolBuffer.h"
#include <functional>
#include <deque>

namespace Mona {


class PersistentData : private Startable, public virtual Object {
public:
	PersistentData(const PoolBuffers& poolBuffers,const char* name = "PersistentData") : _poolBuffers(poolBuffers),Startable(name), _disableTransaction(false) {}

	typedef std::function<void(const std::string& path, const UInt8* value, UInt32 size)> ForEach;

	void load(Exception& ex, const std::string& rootPath, const ForEach& forEach, bool disableTransaction=false);

	bool add(Exception& ex,const std::string& path, const UInt8* value, UInt32 size);
	bool remove(Exception& ex, const std::string& path);

	void flush() { stop(); }
	bool writing() { return running(); }

private:
	class Entry : public virtual Object {
	public:
		Entry(const PoolBuffers& poolBuffers,const std::string& path) : pBuffer(poolBuffers),path(path), toRemove(true) {} // remove
		Entry(const PoolBuffers& poolBuffers,const std::string& path, const UInt8* value, UInt32 size) : path(path), pBuffer(poolBuffers,size), toRemove(false) { // add
			memcpy(pBuffer->data(), value, size);
		}
		PoolBuffer		pBuffer;
		std::string		path;
		bool			toRemove;
	};


	void run(Exception& ex);
	void processEntry(Exception& ex, Entry& entry);
	bool loadDirectory(Exception& ex, const std::string& directory, const std::string& path, const ForEach& forEach);

	std::string			_rootPath;
	std::mutex			_mutex;
	std::deque<Entry>	_entries;
	bool				_disableTransaction;
	const PoolBuffers&	_poolBuffers;
};

} // namespace Mona
