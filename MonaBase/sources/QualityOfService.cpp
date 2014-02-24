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

#include "Mona/QualityOfService.h"

using namespace std;

namespace Mona {


QualityOfService QualityOfService::Null;

QualityOfService::QualityOfService() : lostRate(0),byteRate(0),latency(0),_num(0),_den(0),_size(0) {
}


QualityOfService::~QualityOfService() {
	reset();
}

void QualityOfService::add(UInt32 ping,UInt32 size,UInt32 success,UInt32 lost) {

	(UInt32&)latency = ping/2;

	_num += lost;
	_den += (lost+success);
	_size += size;

	while(!_samples.empty()) {
		Sample& sample(_samples.front());
		if(!sample.time.isElapsed(5000)) // 5 secondes
			break;
		_den -= (sample.success+sample.lost);
		_num -= sample.lost;
		_size -= sample.size;
		_samples.pop_front();
	}
	_samples.emplace_back(success,lost,size);
	
	double elapsed = (double)(*_samples.begin()).time.elapsed();

	(double&)byteRate = (double)_size;
	if(elapsed>0)
		(double&)byteRate = _size/elapsed*1000;

	(double&)lostRate = 0;
	if(_den>0)
		(double&)lostRate = _num/(double)_den;
}

void QualityOfService::reset() {
	(double&)lostRate = 0;
	(double&)byteRate = 0;
	(UInt32&)latency = 0;
	_size=_num=_den=0;
	_samples.clear();
}


} // namespace Mona
