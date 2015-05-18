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
#include "Mona/Time.h"
#include <deque>

namespace Mona {

class QualityOfService : public virtual Object {
public:
	QualityOfService();

	void add(UInt32 size, UInt16 ping,double lostRate = 0); // sending
	void add(double lostRate); // ack
	void reset();

	const double		lostRate;
	const double		byteRate;
	const UInt32		latency;
	Time				lastSendingTime;

	static QualityOfService Null;
private:

	class SendSample : public virtual Object {
		public:
			SendSample(UInt32 size, UInt16 ping) : latency(ping>>1),size(size) {}
			Time	time;
			UInt32	size;
			UInt16	latency;
	};

	class LostSample : public virtual Object {
		public:
			LostSample(double lost) : lost(lost) {}
			Time	time;
			double	lost;
	};

	std::deque<SendSample>	_sendSamples;
	std::deque<LostSample>	_lostSamples;

	UInt32				_size;
	UInt32				_latency;
	UInt32				_latencyCount;
	double				_lost;

	const UInt32		_sampleInterval;
};


} // namespace Mona
