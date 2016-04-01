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

#include "Mona/FileWatcher.h"

namespace Mona {

using namespace std;


bool FileWatcher::watchFile() {
	Time lastModified(file.lastModified(true));
	if (lastModified != _lastModified) { // if path doesn't exist filePath.lastModified()==0
		if (_lastModified)
			clearFile();
		_lastModified.update(lastModified);
		if (lastModified)
			loadFile();
	}
	return lastModified ? true : false;
}

} // namespace Mona
