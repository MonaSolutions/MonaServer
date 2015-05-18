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

QualityOfService::QualityOfService() : lastSendingTime(0),_sampleInterval(4000),lostRate(0),byteRate(0),latency(0),_lost(0),_size(0),_latency(0),_latencyCount(0) {
}

void QualityOfService::reset() {
	(double&)lostRate = 0;
	(double&)byteRate = 0;
	(UInt32&)latency = 0;
	_latencyCount = _latency = _size = 0;
	_lost = 0;
	_sendSamples.clear();
	_lostSamples.clear();
	lastSendingTime = 0;
}

void QualityOfService::add(double lost) {

	while(!_lostSamples.empty()) {
		LostSample& sample(_lostSamples.front());
		if(!sample.time.isElapsed(_sampleInterval))
			break;
		_lost -= sample.lost;
		_lostSamples.pop_front();
	}

	_lost += lost;
	_lostSamples.emplace_back(lost);

	(double&)lostRate = _lost / _lostSamples.size();
}

void QualityOfService::add(UInt32 size, UInt16 ping, double lost) {

	add(lost);

	while(!_sendSamples.empty()) {
		SendSample& sample(_sendSamples.front());
		if(!sample.time.isElapsed(_sampleInterval))
			break;
		_size -= sample.size;
		if (sample.latency) {
			_latency -= sample.latency;
			--_latencyCount;
		}
		_sendSamples.pop_front();
	}

	_size += size;
	_sendSamples.emplace_back(size,ping);
	
	(double&)byteRate = (double)_size;
	double elapsed = (double)_sendSamples.front().time.elapsed();
	if (elapsed > 0)
		(double&)byteRate = (_size / elapsed) * 1000;

	SendSample& sample(_sendSamples.back());
	if (sample.latency) {
		_latency += sample.latency;
		++_latencyCount;
		(UInt32&)latency = _latency / _latencyCount;
	}

	if (size>0)
		lastSendingTime.update();
}


} // namespace Mona
