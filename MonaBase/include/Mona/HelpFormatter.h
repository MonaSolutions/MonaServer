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
#include "Mona/Options.h"
#include <ostream>

namespace Mona {

class HelpFormatter : public virtual Object {
public:
	HelpFormatter(const Options& options) : _options(options), _indent(calcIndent()) {}
	virtual ~HelpFormatter() {}

	std::string command;
	std::string usage;
	std::string header;
	std::string footer;

	void flush(std::ostream& ostr) const;
		/// Writes the formatted help text to the given stream.

private:

	int calcIndent() const;
	/// Calculates the indentation for the option descriptions
	/// from the given options.

	void formatOptions(std::ostream& ostr) const;
	/// Formats all options.

	void formatOption(std::ostream& ostr, const Option& option) const;
	/// Formats an option, using the platform-specific
	/// prefixes.

	void formatText(std::ostream& ostr, const std::string& text, int indent) const;
	/// Formats the given text.

	void flushWord(std::ostream& ostr, int& pos, std::string& word, int indent) const;
	/// Formats the given word.

	const Options&  _options;
	int				_indent;
};



} // namespace Mona
