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
#include <list>

namespace Mona {

class Sample;
class QualityOfService : virtual Object {
public:
	QualityOfService();
	virtual ~QualityOfService();

	void add(UInt32 ping,UInt32 size,UInt32 success=0,UInt32 lost=0);
	void reset();

	const double		lostRate;
	const double		byteRate;
	const UInt32	latency;

	static QualityOfService Null;
private:
	std::list<Sample*>	_samples;
	UInt32		_size;
	UInt32		_num;
	UInt32		_den;
};


} // namespace Mona
