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
#include <list>

namespace Mona {

class Sample;
class QualityOfService {
public:
	QualityOfService();
	virtual ~QualityOfService();

	void add(Mona::UInt32 ping,Mona::UInt32 size,Mona::UInt32 success=0,Mona::UInt32 lost=0);
	void reset();

	const double		lostRate;
	const double		byteRate;
	const Mona::UInt32	latency;

	static QualityOfService Null;
private:
	std::list<Sample*>	_samples;
	Mona::UInt32		_size;
	Mona::UInt32		_num;
	Mona::UInt32		_den;
};


} // namespace Mona
