/*
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 *
 *  This file is part of crtmpserver.
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "Mona/Exceptions.h"
#include "Mona/String.h"

namespace Mona {

void Exception::reset() {
	_code = NIL;
	_error.clear();
}

template <typename ...Args>
void Exception::set(Code code, const Args&... args) {
	_code = code;
	String::Append(_error, args ...);
}

void Exception::set(const Exception& other) {
	_code = other._code;
	_error = other._error;
}

} // namespace Mona

