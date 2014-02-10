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
	if (!_delay.isElapsed(1500000)) // already checked there is less of 1.5 sec!
		return _exists;
	_delay.update();
	filePath.update();
	if (filePath.lastModified() != _lastModified) { // if path doesn't exist filePath.lastModified()==0
		if (_lastModified > 0) {
			_exists = false;
			clearFile();
		}
		_lastModified.update(filePath.lastModified());
		if (filePath.lastModified() > 0) {
			_exists = true;
			loadFile();
		}
	}
	return _exists;
}

} // namespace Mona
