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

#include "FileWatcher.h"

using namespace std;
using namespace Poco;
using namespace Mona;


FileWatcher::FileWatcher(const string& path) : _file(path), path(path) {
	_lastModified.update(0);
}


FileWatcher::~FileWatcher() {

}

bool FileWatcher::watch() {
	bool result=false;
	if(_lastModified>0) {
		Mona::Int64 lastModif = _file.getLastModified().epochMicroseconds();
		if (!_file.exists() || !_file.isFile() || lastModif > _lastModified) {
			_lastModified.update(lastModif);
			clear();
			if(_lastModified>0) {
				load();
				result=true;
			}
		}
	} else if(_file.exists() && _file.isFile()) {
		_lastModified.update(_file.getLastModified().epochMicroseconds());
		load();
		result=true;
	}
	return result;
}
