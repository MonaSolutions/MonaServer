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

#pragma once

#include "Mona/Mona.h"
#include <functional>

namespace Mona {


class Option {
	friend class Options;
public:
	Option();

	Option(const std::string& fullName, const std::string& shortName);
		/// Creates an option with the given properties.

	Option(const std::string& fullName, const std::string& shortName, const std::string& description, bool required = false);
		/// Creates an option with the given properties.

	Option(const std::string& fullName, const std::string& shortName, const std::string& description, bool required, const std::string& argName, bool argRequired = false);
		/// Creates an option with the given properties.

	template<class Function>
	Option& handler(const Function& function) {
		_handler = function;
		return *this;
	}
	
	Option& description(const std::string& text);
		/// Sets the description of the option.
		
	Option& required(bool flag);
		/// Sets whether the option is required (flag == true)
		/// or optional (flag == false).

	Option& repeatable(bool flag);
		/// Sets whether the option can be specified more than once
		/// (flag == true) or at most once (flag == false).
		
	Option& argument(const std::string& name, bool required = true);
		/// Specifies that the option takes an (optional or required)
		/// argument.
		
	Option& noArgument();
		/// Specifies that the option does not take an argument (default).

	const std::string& shortName() const;
		/// Returns the short name of the option.
		
	const std::string& fullName() const;
		/// Returns the full name of the option.
		
	const std::string& description() const;
		/// Returns the description of the option.
		
	bool required() const;
		/// Returns true if the option is required, false if not.
	
	bool repeatable() const;
		/// Returns true if the option can be specified more than
		/// once, or false if at most once.
	
	bool takesArgument() const;
		/// Returns true if the options takes an (optional) argument.
		
	bool argumentRequired() const;
		/// Returns true if the argument is required.

	const std::string& argumentName() const;
		/// Returns the argument name, if specified.


	bool operator==(const Option& other) const;
	bool operator!=(const Option& other) const;
	bool operator<(const Option& other) const;
	bool operator>(const Option& other) const;

private:
	std::string		_shortName;
	std::string		_fullName;
	std::string		_description;
	bool			_required;
	bool			_repeatable;
	std::string		_argName;
	bool			_argRequired;

	std::function<void(const std::string& value)>	_handler;
};


//
// inlines
//

inline bool Option::operator==(const Option& other) const {
	return _fullName == other._fullName || _shortName == other._shortName;
}
inline bool Option::operator!=(const Option& other) const {
	return _fullName != other._fullName && _shortName != other._shortName;
}
inline bool Option::operator<(const Option& other) const {
	return _fullName < other._fullName;
}
inline bool Option::operator>(const Option& other) const {
	return _fullName > other._fullName;
}

inline const std::string& Option::shortName() const {
	return _shortName;
}
inline const std::string& Option::fullName() const {
	return _fullName;
}
inline const std::string& Option::description() const {
	return _description;
}
inline bool Option::required() const {
	return _required;
}
inline bool Option::repeatable() const {
	return _repeatable;
}
inline bool Option::takesArgument() const {
	return !_argName.empty();
}	
inline bool Option::argumentRequired() const {
	return _argRequired;
}
inline const std::string& Option::argumentName() const {
	return _argName;
}


} // namespace Mona
