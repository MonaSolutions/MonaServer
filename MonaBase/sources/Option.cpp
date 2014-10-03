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

#include "Mona/Option.h"

using namespace std;

namespace Mona {

Option Option::Null(NULL,NULL);


Option::Option(const char* fullName, const char* shortName) :
	_shortName(shortName ? shortName : ""),
	_fullName(fullName ? fullName : ""),
	_required(false),
	_repeatable(false),
	_argRequired(false) {
}


Option::Option(const char* fullName, const char* shortName, const string& description, bool required) :
	_shortName(shortName ? shortName : ""),
	_fullName(fullName ? fullName : ""),
	_description(description),
	_required(required),
	_repeatable(false),
	_argRequired(false) {
}


Option::Option(const char* fullName, const char* shortName, const string& description, bool required, const string& argName, bool argRequired) :
	_shortName(shortName),
	_fullName(fullName),
	_description(description),
	_required(required),
	_repeatable(false),
	_argName(argName),
	_argRequired(argRequired) {
}

Option& Option::handler(const Handler& function) {
	_handler = function;
	return *this;
}

Option& Option::description(const string& text) {
	_description = text;
	return *this;
}

	
Option& Option::required(bool flag) {
	_required = flag;
	return *this;
}


Option& Option::repeatable(bool flag) {
	_repeatable = flag;
	return *this;
}

	
Option& Option::argument(const string& name, bool required) {
	_argName     = name;
	_argRequired = required;
	return *this;
}

	
Option& Option::noArgument() {
	_argName.clear();
	_argRequired = false;
	return *this;
}



} // namespace Mona
