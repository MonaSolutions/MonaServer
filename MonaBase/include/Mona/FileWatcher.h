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
#include "Mona/File.h"

namespace Mona {

class FileWatcher : public virtual Object {
public:

	template <typename ...Args>
	FileWatcher(Args&&... args) : file(args ...), _lastModified(0) {}
	

	/// look if the file has changed, call clearFile if doesn't exist anymore, or call clearFile and loadFile if file has change
	/// return true if file exists
	bool		watchFile();

	const File	file;

private:
	virtual void loadFile() = 0;
	virtual void clearFile() = 0;

	Time		_lastModified;
};

} // namespace Mona
