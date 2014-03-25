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
#include "Mona/Date.h"
#include "Mona/BinaryReader.h"
#include <set>

#ifdef _WIN32
struct _SYSTEMTIME;
#endif

namespace Mona {

class Timezone : public virtual Object {
public:

	enum TimeType {
		DST,
		STANDARD,
		CHECK_DST_WITH_RULES
	};

	operator			Int32() const	{ return _offset; } // timezone in ms
	Int32				offset()const	{ return _offset; } // timezone in ms
	const std::string&  name()  const   { return _name; }

	static Timezone&	Local() { return _Timezone; }

	static Int32		LocalOffset(const Date& date, bool& isDST) { return _Timezone.localOffset(date,date.clock(),isDST); }
	static Int32		LocalOffset(const Date& date, UInt32 clock, bool& isDST) { return _Timezone.localOffset(date,clock,isDST); }
	static Int32		LocalOffsetUsingRules(const Date& date, bool& isDST) { return _Timezone.localOffsetUsingtRules(date,date.clock(),isDST); }
	static Int32		LocalOffsetUsingRules(const Date& date, UInt32 clock, bool& isDST) { return _Timezone.localOffsetUsingtRules(date,clock,isDST); }
	static Int32		LocalOffset(Int64 time, TimeType& timeType) { return _Timezone.localOffset(time,timeType); }

	static Int32		Offset(const std::string& code) { bool isDST(false);  return Offset(code, isDST); }
	static Int32		Offset(const std::string& code,bool& isDST);


private:
	struct TransitionRule {
		enum Type {
			NIL,
			ABSOLUTE,
			JULIAN,
			RELATIVE
		};
		TransitionRule() : type(NIL),month(0),day(0),week(0),clock(2*3600000) {}
		UInt8	month; // 1 to 12
		UInt8	week; // 1 to 5
		UInt16	day; // 0 to 6 if type==RELATIVE, 0 to 365 if type==ABSOLUTE (leap counted), 1 to 365 if type==JULIAN (no leap counted)
		UInt32	clock; // clock in ms
		Type	type;
		operator bool() const { return type!=NIL; }
	};

	class Transition : public virtual Object {
	public:
		Transition(Int32 offset,bool isDST) : offset(offset),isDST(isDST) {}
		const Int32  offset;
		const bool   isDST;
	};

	
	Timezone();

	Int32 localOffset(const Date& date,UInt32 clock,bool& isDST);
	Int32 localOffsetUsingtRules(const Date& date,UInt32 clock,bool& isDST);

	Int32 localOffset(Int64 time, TimeType& timeType);


	bool  isBeforeTransition(const Date& date,UInt32 clock,const TransitionRule& rule);
#if defined(_WIN32)
	void  fillDefaultTransitionRule(_SYSTEMTIME& date, Timezone::TransitionRule& rule, Int32 offset,Int32 dstOffset, bool isDST);
#endif

	Int32			   _offset;
	Int32			   _dstOffset;
	std::string		   _name;

	static TransitionRule					_StartDST;
	static TransitionRule					_EndDST;
	static std::map<Int64,Transition>		_Transitions;
	static std::map<Int64,Transition>		_LocalTransitions;
	static Timezone							_Timezone;
	

	/// TZ Database
	static bool  ParseTZRule(std::string::const_iterator& it, const std::string::const_iterator& end, TransitionRule& rule);
	static Int32 ParseTZOffset(std::string::const_iterator& it, const std::string::const_iterator& end, Int32 defaultValue);


	bool  readTZDatabase(const std::string& path);
	
	template <typename Type>
	bool readTZData(BinaryReader& reader) {
		if (reader.available() < 39)
			return false;

		reader.next(15); // skip header

		UInt32 gmtSize(reader.read32());
		UInt32 stdSize(reader.read32());
		UInt32 leapSize(reader.read32());
		UInt32 timeSize(reader.read32());
		UInt32 typeSize(reader.read32());
		UInt32 charSize(reader.read32());
		UInt32 i(0);

		UInt32 position(reader.position());
		reader.next(timeSize*(sizeof(Type)+1));

		// Types
		struct TransitionType {
			TransitionType() : offset(7200000), isDST(false) {}
			Int32 offset;
			bool isDST;
		};

		if (reader.available() < (typeSize*6))
			return false;

		std::vector<TransitionType> types(typeSize);
		for (i = 0; i < typeSize; ++i) {
			types[i].offset = reader.read32();
			types[i].isDST = reader.read8()!=0;
			reader.next(1); // skip abbr (pointer to name)
		}

		 // Skip charSize
		 reader.next(charSize);
	 
		// Times (sorted in ascending time order)
		UInt32 end(reader.position());
		reader.reset(position);
		UInt32 size(timeSize*sizeof(Type));
		Int32 stdOffset(_offset);
		BinaryReader typeReader(reader.current() + size,reader.available()-size);
		for (i = 0; i < timeSize; ++i) {
			UInt8 id(typeReader.read8());
			if (id >= types.size()) {
				reader.next(sizeof(Type));
				continue;
			}
			Int64 time(sizeof(Type) == 4 ? (Int32)reader.read32() : reader.read64());
			time *= 1000;
			bool isDST(types[id].isDST);
			Int32 offset(types[id].offset*1000);
			_Transitions.emplace(std::piecewise_construct, std::forward_as_tuple(time),std::forward_as_tuple(offset,isDST));
			if (!isDST)
				time += (stdOffset = offset);
			else
				time += stdOffset;
			_LocalTransitions.emplace(std::piecewise_construct, std::forward_as_tuple(time),std::forward_as_tuple(offset,isDST));
		}
		reader.reset(end);

		// Skip leap seconds
		reader.next(leapSize*2*sizeof(Type));

		// Skip stdSize and gmtSize
		reader.next(stdSize);
		reader.next(gmtSize);

		/*
		// display result
		for (const Transition& transition : _Transitions) {
			Date date;
			TimeToDate(transition, date);
			printf("%d/%hhu/%hhu @ %hhu:%hhu:%hhu,%hu =>",date.year,date.month,date.day,date.hour,date.minute,date.second,date.millisecond);
			printf(" %d | %s\n", transition.gmtOffset,transition.isDST ? "DST" : "STD");
		}*/
		return true;
	}

};


} // namespace Mona

