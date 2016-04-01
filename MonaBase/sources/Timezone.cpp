/*
Copyright 2014 Mona
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3*3600000 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

This file is a part of Mona.
*/

#include "Mona/Timezone.h"
#include "Mona/Util.h"
#include "Mona/File.h"
#include <fstream>
#if defined(_WIN32)
#include "windows.h"
#else
   #if !defined(_OS_BSD)
      extern long timezone;
   #endif
   extern char *tzname[2];
#endif

#undef ABSOLUTE
#undef RELATIVE

using namespace std;

namespace Mona {

static const map<string, pair<Int32, bool>> Timezones({
	{ "ACDT", { 10*3600000 + 1800000, true } },
	{ "ACST", { 9*3600000 + 1800000, false } },
	{ "ADT", { -3*3600000, true } },
	{ "AEDT", { 11*3600000, true } },
	{ "AEST", { 10*3600000, false } },
	{ "AFT", { 4*3600000 + 1800000, false } },
	{ "AKDT", { -8*3600000, true } },
	{ "AKST", { -9*3600000, false } },
	{ "ALMT", { 6*3600000, false } },
	{ "AMT", { 4*3600000, false } },
	{ "AMST", { 5*3600000, true } },
	{ "ANAT", { 11*3600000, false } },
	{ "ANAST", { 12*3600000, true } },
	{ "AQTT", { 5*3600000, false } },
	{ "ART", { -3*3600000, false } },
	{ "AST", { -4*3600000, false } },
	{ "AWDT", { 9*3600000, true } },
	{ "AWST", { 8*3600000, false } },
	{ "AZOT", { -3600000, false } },
	{ "AZOST", { 0, true } },
	{ "AZT", { 4*3600000, false } },
	{ "AZST", { 5*3600000, true } },
	{ "BNT", { 8*3600000, false } },
	{ "BDT", { 6*3600000, false } },
	{ "BOT", { -4*3600000, false } },
	{ "BRT", { -3*3600000, false } },
	{ "BRST", { -2*3600000, true } },
	{ "BST", { 3600000, true } },
	{ "BTT", { 6*3600000, false } },
	{ "CAST", { 2*3600000, false } },
	{ "CAT", { 2*3600000, false } },
	{ "CCT", { 6*3600000 + 1800000, false } },
	{ "CDT", { -5*3600000, true } },
	{ "CEDT", { 2*3600000, true } },
	{ "CEST", { 2*3600000, true } },
	{ "CET", { 3600000, false } },
	{ "CHADT", { 13*3600000 + 270000, true } },
	{ "CHAST", { 12*3600000 + 270000, false } },
	{ "CHOT", { 8*3600000, false } },
	{ "CHOST", { 9*3600000, true } },
	{ "CHST", { 10*3600000, false } },
	{ "CIT", { 8*3600000, false } },
	{ "CKT", { -10*3600000, false } },
	{ "CLST", { -3*3600000, true } },
	{ "CLT", { -4*3600000, false } },
	{ "COT", { -5*3600000, false } },
	{ "CST", { -6*3600000, false } },
	{ "CT", { 8*3600000, false } },
	{ "CVT", { -3600000, false } },
	{ "CWST", { 8*3600000 + 270000, false } },
	{ "CXT", { 7*3600000, false } },
	{ "DAVT", { 7*3600000, false } },
	{ "DDUT", { 10*3600000, false } },
	{ "EASST", { -5*3600000, true } },
	{ "EAST", { -6*3600000, false } },
	{ "EAT", { 3*3600000, false } },
	{ "ECT", { -5*3600000, false } },
	{ "EDT", { -4*3600000, true } },
	{ "EEDT", { 3*3600000, true } },
	{ "EEST", { 3*3600000, true } },
	{ "EET", { 2*3600000, false } },
	{ "EGT", { -3600000, false } },
	{ "EGST", { 0, true } },
	{ "EST", { -5*3600000, false } },
	{ "EIT", { 9*3600000, false } },
	{ "FJT", { 12*3600000, false } },
	{ "FJST", { 13*3600000, true } },
	{ "FKST", { -3*3600000, true } },
	{ "FKT", { -4*3600000, false } },
	{ "FNT", { -2*3600000, false } },
	{ "GALT", { -6*3600000, false } },
	{ "GAMT", { 9*3600000, false } },
	{ "GFT", { -3*3600000, false } },
	{ "GILT", { 12*3600000, false } },
	{ "GST", { 4*3600000, false } },
	{ "GYT", { -4*3600000, false } },
	{ "HADT", { -9*3600000, true } },
	{ "HAST", { -10*3600000, false } },
	{ "HKT", { 8*3600000, false } },
	{ "HOVT", { 7*3600000, false } },
	{ "HOVST", { 8*3600000, true } },
	{ "HST", { -10*3600000, false } },
	{ "ICT", { 7*3600000, false } },
	{ "IDT", { 3*3600000, true } },
	{ "IOT", { 6*3600000, false } },
	{ "IRDT", { 4*3600000 + 1800000, true } },
	{ "IRKT", { 8*3600000, false } },
	{ "IRKST", { 9*3600000, true } },
	{ "IRST", { 3*3600000 + 1800000, false } },
	{ "IST", { 3600000, true } },
	{ "JST", { 9*3600000, false } },
	{ "KGT", { 6*3600000, false } },
	{ "KOST", { 11*3600000, false } },
	{ "KRAT", { 7*3600000, false } },
	{ "KRAST", { 8*3600000, true } },
	{ "KST", { 9*3600000, false } },
	{ "KUYT", { 4*3600000, false } },
	{ "LHDT", { 11*3600000, true } },
	{ "LHST", { 10*3600000 + 1800000, false } },
	{ "LINT", { 14*3600000, false } },
	{ "LKT", { 5*3600000 + 1800000, false } },
	{ "MAGT", { 11*3600000, false } },
	{ "MAGST", { 12*3600000, true } },
	{ "MART", { -9*3600000 - 1800000, false } },
	{ "MAWT", { 5*3600000, false } },
	{ "MDT", { -6*3600000, true } },
	{ "MeST", { -8*3600000, false } },
	{ "MHT", { 12*3600000, false } },
	{ "MIST", { 11*3600000, false } },
	{ "MMT", { 6*3600000 + 1800000, false } },
	{ "MSD", { 4*3600000, true } },
	{ "MSK", { 3*3600000, false } },
	{ "MST", { -7*3600000, false } },
	{ "MUT", { 4*3600000, false } },
	{ "MVT", { 5*3600000, false } },
	{ "MYT", { 8*3600000, false } },
	{ "NCT", { 11*3600000, false } },
	{ "NDT", { -2*3600000 - 1800000, true } },
	{ "NFT", { 11*3600000 + 1800000, false } },
	{ "NOVT", { 6*3600000, false } },
	{ "NOVST", { 7*3600000, true } },
	{ "NPT", { 5*3600000 + 270000, false } },
	{ "NRT", { 12*3600000, false } },
	{ "NST", { -3*3600000 - 1800000, false } },
	{ "NT", { -3*3600000 - 1800000, false } },
	{ "NUT", { -11*3600000, false } },
	{ "NZDT", { 13*3600000, true } },
	{ "NZST", { 12*3600000, false } },
	{ "OMST", { 6*3600000, false } },
	{ "OMSST", { 7*3600000, true } },
	{ "ORAT", { 5*3600000, false } },
	{ "PDT", { -7*3600000, true } },
	{ "PET", { -5*3600000, false } },
	{ "PETT", { 12*3600000, false } },
	{ "PETST", { 13*3600000, true } },
	{ "PGT", { 10*3600000, false } },
	{ "PHT", { 8*3600000, false } },
	{ "PHOT", { 13*3600000, false } },
	{ "PKT", { 5*3600000, false } },
	{ "PMDT", { -2*3600000, true } },
	{ "PMST", { -3*3600000, false } },
	{ "PONT", { 11*3600000, false } },
	{ "PST", { -8*3600000, false } },
	{ "PWT", { 9*3600000, false } },
	{ "PYT", { -4*3600000, false } },
	{ "PYST", { -3*3600000, true } },
	{ "QYZT", { +6*3600000, false } },
	{ "RET", { 4*3600000, false } },
	{ "ROTT", { -3*3600000, false } },
	{ "SAKT", { 11*3600000, false } },
	{ "SAKST", { 12*3600000, true } },
	{ "SAMT", { 4*3600000, false } },
	{ "SAST", { 2*3600000, false } },
	{ "SBT", { 11*3600000, false } },
	{ "SCT", { 4*3600000, false } },
	{ "SGT", { 8*3600000, false } },
	{ "SRT", { -3*3600000, false } },
	{ "SLT", { 5*3600000 + 1800000, false } },
	{ "SST", { -11*3600000, false } },
	{ "SYOT", { 3*3600000, false } },
	{ "TAHT", { -10*3600000, false } },
	{ "TFT", { 5*3600000, false } },
	{ "TJT", { 5*3600000, false } },
	{ "TKT", { -10*3600000, false } },
	{ "TLT", { 9*3600000, false } },
	{ "TMT", { 5*3600000, false } },
	{ "TOT", { 13*3600000, false } },
	{ "TRUT", { 10*3600000, false } },
	{ "TVT", { 12*3600000, false } },
	{ "ULAT", { 8*3600000, false } },
	{ "ULAST", { 9*3600000, true } },
	{ "UYST", { -2*3600000, true } },
	{ "UYT", { -3*3600000, false } },
	{ "UZT", { 5*3600000, false } },
	{ "VET", { -4*3600000 - 1800000, false } },
	{ "VLAT", { 10*3600000, false } },
	{ "VLAST", { 11*3600000, true } },
	{ "VOLT", { 4*3600000, false } },
	{ "VOST", { 6*3600000, false } },
	{ "VUT", { 11*3600000, false } },
	{ "WAKT", { 12*3600000, false } },
	{ "WAT", { 3600000, false } },
	{ "WART", { -4*3600000, false } },
	{ "WAST", { 2*3600000, true } },
	{ "WDT", { 9*3600000, true } },
	{ "WEDT", { 3600000, true } },
	{ "WEST", { 3600000, true } },
	{ "WET", { 0, false } },
	{ "WFT", { 12*3600000, false } },
	{ "WGT", { -3*3600000, false } },
	{ "WGST", { -2*3600000, true } },
	{ "WIT", { 7*3600000, false } },
	{ "WST", { 8*3600000, false } },
	{ "WT", { 0, false } },
	{ "YAKT", { 9*3600000, false } },
	{ "YAKST", { 10*3600000, true } },
	{ "YEKT", { 5*3600000, false } },
	{ "YEKST", { 6*3600000, true } }
});

Int32 Timezone::ParseTZOffset(string::const_iterator& it,const string::const_iterator& end,Int32 defaultValue) {
	if (it == end)
		return defaultValue;
	bool ahead(false);
	if (*it == '-') { ahead = true; ++it; }
	if (it == end)
		return defaultValue;
	Int32 result(0);
	bool gotten(false);
	while (it != end && isdigit(*it)) { result = result * 10 + 3600000 * ((*it) - '0'); ++it; gotten = true; } // hour
	if (!gotten)
		return defaultValue;
	Int32 temp(0);
	if (it != end && *it == ':') { ++it;  while (it != end && isdigit(*it)) { temp = temp*10 + 60000 * ((*it)-'0'); ++it; } } // minute
	result += temp;
	if (it != end && *it == ':') { temp=0; ++it;  while (it != end && isdigit(*it)) { temp = temp*10 + 1000 * ((*it)-'0'); ++it; } } // second
	result += temp;
	if (ahead)
		return result;
	return -result;
}

bool Timezone::ParseTZRule(string::const_iterator& it,const string::const_iterator& end,TransitionRule& rule) {
	if (it == end)
		return false;
	TransitionRule::Type type(TransitionRule::NIL);
	UInt8 month(0), week(0), day(0);
	if (*it == 'J' && ++it!=end) {
		while (it!=end && isdigit(*it)) { day = day*10 + ((*it)-'0'); ++it;} // julian day (1 to 365 without leap counted)
		if (day>0)
			type = TransitionRule::JULIAN;
	} else if (*it == 'M' && ++it!=end) {
		while (it != end && isdigit(*it)) { month = month*10 + ((*it)-'0'); ++it; } // month
		if (it != end && *it == '.') { ++it;  while (it != end && isdigit(*it)) { week = week*10 + ((*it)-'0'); ++it; } } // week
		if (it != end && *it == '.') { ++it;  while (it != end && isdigit(*it)) { day = day*10 + ((*it)-'0'); ++it; } } // day
		if (month>0 && week>0)
			type = TransitionRule::RELATIVE;
	} else {
		while (it!=end && isdigit(*it)) { day = day*10 + ((*it)-'0'); ++it;} // absolute day (0 to 365 with leap counted)
		if (day>0)
			type = TransitionRule::ABSOLUTE;
	}
	if (type==TransitionRule::NIL)
		return false;
	rule.type = type; rule.month = month; rule.week = week; rule.day = day;
	if (it != end && *it == '/') {
		bool gotten(false);
		month = week = day = 0;
		++it;
		while (it != end && isdigit(*it)) { month = month * 10 + ((*it) - '0'); ++it; gotten = true; } // hour
		if (!gotten) {
			rule.clock = month*3600000;
			return true;
		}
		if (it != end && *it == ':') { ++it;  while (it != end && isdigit(*it)) { week = week*10 + ((*it)-'0'); ++it; } } // minute
		rule.clock += week*60000;
		if (it != end && *it == ':') { ++it;  while (it != end && isdigit(*it)) { day = day*10 + 1000 * ((*it)-'0'); ++it; } } // second
		rule.clock += day*1000;

	}
	return true;
}


bool Timezone::readTZDatabase(const string& path) {
	ifstream ifile(path.c_str(), ios::in | ios::binary | ios::ate);
	if (!ifile.good())
		return false;
	
	UInt32 size = (UInt32)ifile.tellg();
	if (size < 4)
		return false;
	vector<UInt8> buffer(size);
	ifile.seekg(0);
	ifile.read((char*)buffer.data(), size);

	BinaryReader reader(buffer.data(),buffer.size());

	reader.next(4);
	if (memcmp(reader.data(), "TZif", 4) != 0)
		return false;

	char version(reader.read8());

	if (readTZData<Int32>(reader) && version >= '2' && reader.available()>4 && memcmp(reader.current(), "TZif", 4) == 0) {
		reader.next(5); // skip TZif2
		readTZData<Int64>(reader);
	}

	reader.next(1); // '\n'


	// Default Rule (for superior date) => http://man7.org/linux/man-pages/man3/tzset.3.html
	// std offset dst [offset],start[/time],end[/time]
	// NZST-12:00:00NZDT-13:00:00,M10.1.0,M3.3.0
	// CET-1CEST,M3.5.0,M10.5.0/3
	if (reader.available() < 2)
		return false;
	((UInt8*)reader.current())[reader.available() - 1] = '\0'; // to be sure that there is a null-terminated character before the end
	string value((const char*)reader.current());
	string::const_iterator it(value.begin()),end(value.end());
	while (it!=end && isalpha(*it)) ++it; // skip std
	_offset = ParseTZOffset(it,end,_offset);
	while (it!=end && isalpha(*it)) ++it; // skip dst
	_dstOffset = ParseTZOffset(it,end,_dstOffset);
	if (it != end) ++it; // skip ,
	if (ParseTZRule(it, end, _StartDST)) {
		if (it != end) ++it; // skip ,
		ParseTZRule(it, end, _EndDST);
	}
	return true;
}


#if defined(_WIN32)
void Timezone::fillDefaultTransitionRule(const SYSTEMTIME& date,Int32 offset,Int32 dstOffset,bool isDST,Timezone::TransitionRule& rule) {
	if (date.wYear > 0) {
		// it's a Transition (so no default rule... no daylight excepting for this year)
		Date transitionDate((Int32)date.wYear, (UInt8)date.wMonth, (UInt8)date.wDay, (UInt8)date.wHour, (UInt8)date.wMinute, (UInt8)date.wSecond,0,Date::GMT);
		_Transitions[(transitionDate-offset)].set(_LocalTransitions[transitionDate].set(isDST ? dstOffset : offset,isDST));
		return;
	}
		
		// absolute
	rule.month = (UInt8)date.wMonth;
	rule.week = (UInt8)date.wDay;
	rule.day = (UInt16)date.wDayOfWeek;
	rule.clock = date.wHour*3600000L + date.wMinute*60000L + date.wSecond*1000;
	rule.type = TransitionRule::RELATIVE;
}
#endif

Timezone::Timezone() : _offset(0),_dstOffset(3600000) {
#if defined(_WIN32)
	/// GET LOCAL INFOS

	_tzset();

	//// LOAD default values ////////////////////////

	// Following code is used for Windows XP and 2000 compatibility
	typedef DWORD (WINAPI *TimeZoneFct)(PDYNAMIC_TIME_ZONE_INFORMATION);
	TimeZoneFct dynamicTzFunction = (TimeZoneFct) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetDynamicTimeZoneInformation");
	DYNAMIC_TIME_ZONE_INFORMATION	timezoneInfo;
	DWORD result = (dynamicTzFunction)? dynamicTzFunction(&timezoneInfo) : GetTimeZoneInformation((PTIME_ZONE_INFORMATION)&timezoneInfo);

	_offset=(-(Int32)timezoneInfo.Bias*60000);
	if (result != TIME_ZONE_ID_UNKNOWN)
		_dstOffset = (-(Int32)timezoneInfo.DaylightBias*60000);
	_dstOffset += _offset;

	fillDefaultTransitionRule(timezoneInfo.DaylightDate,_offset,_dstOffset,true,_StartDST);
	fillDefaultTransitionRule(timezoneInfo.StandardDate,_offset,_dstOffset,false,_EndDST);

	int size(sizeof(timezoneInfo.TimeZoneKeyName));
	_name.resize(size);
	if (dynamicTzFunction && (size = WideCharToMultiByte(CP_ACP, 0, timezoneInfo.TimeZoneKeyName, -1, (char*)_name.data(), _name.size(), NULL, NULL)) > 0) {
		_name.resize(size-1);

		static const map<string, const char*> WindowToTZID ({
			{ "AUS Central Standard Time", "Australia/Darwin" },
			{ "AUS Eastern Standard Time", "Australia/Sydney" },
			{ "Afghanistan Standard Time", "Asia/Kabul" },
			{ "Alaskan Standard Time", "America/Anchorage" },
			{ "Arab Standard Time", "Asia/Riyadh" },
			{ "Arabian Standard Time", "Asia/Dubai" },
			{ "Arabic Standard Time", "Asia/Baghdad" },
			{ "Argentina Standard Time", "America/Buenos_Aires" },
			{ "Atlantic Standard Time", "America/Halifax" },
			{ "Azerbaijan Standard Time", "Asia/Baku" },
			{ "Azores Standard Time", "Atlantic/Azores" },
			{ "Bahia Standard Time", "America/Bahia" },
			{ "Bangladesh Standard Time", "Asia/Dhaka" },
			{ "Canada Central Standard Time", "America/Regina" },
			{ "Cape Verde Standard Time", "Atlantic/Cape_Verde" },
			{ "Caucasus Standard Time", "Asia/Yerevan" },
			{ "Cen. Australia Standard Time", "Australia/Adelaide" },
			{ "Central America Standard Time", "America/Guatemala" },
			{ "Central Asia Standard Time", "Asia/Almaty" },
			{ "Central Brazilian Standard Time", "America/Cuiaba" },
			{ "Central Europe Standard Time", "Europe/Budapest" },
			{ "Central European Standard Time", "Europe/Warsaw" },
			{ "Central Pacific Standard Time", "Pacific/Guadalcanal" },
			{ "Central Standard Time", "America/Chicago" },
			{ "China Standard Time", "Asia/Shanghai" },
			{ "Dateline Standard Time", "Etc/GMT+12" },
			{ "E. Africa Standard Time", "Africa/Nairobi" },
			{ "E. Australia Standard Time", "Australia/Brisbane" },
			{ "E. Europe Standard Time", "Asia/Nicosia" },
			{ "E. South America Standard Time", "America/Sao_Paulo" },
			{ "Eastern Standard Time", "America/New_York" },
			{ "Egypt Standard Time", "Africa/Cairo" },
			{ "Ekaterinburg Standard Time", "Asia/Yekaterinburg" },
			{ "FLE Standard Time", "Europe/Kiev" },
			{ "Fiji Standard Time", "Pacific/Fiji" },
			{ "GMT Standard Time", "Europe/London" },
			{ "GTB Standard Time", "Europe/Bucharest" },
			{ "Georgian Standard Time", "Asia/Tbilisi" },
			{ "Greenland Standard Time", "America/Godthab" },
			{ "Greenwich Standard Time", "Atlantic/Reykjavik" },
			{ "Hawaiian Standard Time", "Pacific/Honolulu" },
			{ "India Standard Time", "Asia/Calcutta" },
			{ "Iran Standard Time", "Asia/Tehran" },
			{ "Israel Standard Time", "Asia/Jerusalem" },
			{ "Jordan Standard Time", "Asia/Amman" },
			{ "Kaliningrad Standard Time", "Europe/Kaliningrad" },
			{ "Korea Standard Time", "Asia/Seoul" },
			{ "Libya Standard Time", "Africa/Tripoli" },
			{ "Magadan Standard Time", "Asia/Magadan" },
			{ "Mauritius Standard Time", "Indian/Mauritius" },
			{ "Mauritius Standard Time", "Indian/Reunion" },
			{ "Middle East Standard Time", "Asia/Beirut" },
			{ "Montevideo Standard Time", "America/Montevideo" },
			{ "Morocco Standard Time", "Africa/Casablanca" },
			{ "Mountain Standard Time", "America/Denver" },
			{ "Myanmar Standard Time", "Asia/Rangoon" },
			{ "N. Central Asia Standard Time", "Asia/Novosibirsk" },
			{ "Namibia Standard Time", "Africa/Windhoek" },
			{ "Nepal Standard Time", "Asia/Katmandu" },
			{ "New Zealand Standard Time", "Pacific/Auckland" },
			{ "Newfoundland Standard Time", "America/St_Johns" },
			{ "North Asia East Standard Time", "Asia/Irkutsk" },
			{ "North Asia Standard Time", "Asia/Krasnoyarsk" },
			{ "Pacific SA Standard Time", "America/Santiago" },
			{ "Pacific Standard Time", "America/Los_Angeles" },
			{ "Pakistan Standard Time", "Asia/Karachi" },
			{ "Paraguay Standard Time", "America/Asuncion" },
			{ "Romance Standard Time", "Europe/Paris" },
			{ "Russian Standard Time", "Europe/Moscow" },
			{ "SA Eastern Standard Time", "America/Cayenne" },
			{ "SA Pacific Standard Time", "America/Bogota" },
			{ "SA Western Standard Time", "America/La_Paz" },
			{ "SE Asia Standard Time", "Asia/Bangkok" },
			{ "Samoa Standard Time", "Pacific/Apia" },
			{ "Singapore Standard Time", "Asia/Singapore" },
			{ "South Africa Standard Time", "Africa/Johannesburg" },
			{ "Sri Lanka Standard Time", "Asia/Colombo" },
			{ "Syria Standard Time", "Asia/Damascus" },
			{ "Taipei Standard Time", "Asia/Taipei" },
			{ "Tasmania Standard Time", "Australia/Hobart" },
			{ "Tokyo Standard Time", "Asia/Tokyo" },
			{ "Tonga Standard Time", "Pacific/Tongatapu" },
			{ "Turkey Standard Time", "Europe/Istanbul" },
			{ "US Eastern Standard Time", "America/Indianapolis" },
			{ "US Mountain Standard Time", "America/Phoenix" },
			{ "UTC",	"Etc/GMT" },
			{ "UTC+12", "Etc/GMT-12" },
			{ "UTC-02", "Etc/GMT+2" },
			{ "UTC-11", "Etc/GMT+11" },
			{ "Ulaanbaatar Standard Time", "Asia/Ulaanbaatar" },
			{ "Venezuela Standard Time", "America/Caracas" },
			{ "Vladivostok Standard Time", "Asia/Vladivostok" },
			{ "W. Australia Standard Time", "Australia/Perth" },
			{ "W. Central Africa Standard Time", "Africa/Lagos" },
			{ "W. Europe Standard Time", "Europe/Berlin" },
			{ "West Asia Standard Time", "Asia/Tashkent" },
			{ "West Pacific Standard Time", "Pacific/Port_Moresby" },
			{ "Yakutsk Standard Time", "Asia/Yakutsk" },
		});


		auto& it = WindowToTZID.find(_name);
		if (it != WindowToTZID.end())
			_name.assign(it->second);
	} else
		_name.clear();
#else
   tzset();
   #if defined(_OS_BSD) // timezone not available on BSD
      std::time_t now = std::time(NULL);
      struct std::tm t;
      gmtime_r(&now, &t);
      _offset = now - std::mktime(&t);
   #else
      _offset = (-(Int32)timezone*1000);
   #endif
   _name.assign(tzname[0]);
   _dstOffset += _offset;
#endif


   if (_name.empty())
	   return; // no more TZ information available


	//// LOAD TZ database ////////////////////////
    string path;
	if (Util::Environment().getString("TZDIR", path) && readTZDatabase(FileSystem::MakeFolder(path).append(_name)))
		return;

#if !defined(_WIN32)

    path.assign("/usr/lib/zoneinfo/").append(_name);
	if(readTZDatabase(path))
		return;

	path.assign("/usr/share/zoneinfo/").append(_name);
	if(readTZDatabase(path))
		return;

#endif

	if (FileSystem::GetCurrentDir(path) && readTZDatabase(path.append("zoneinfo/").append(_name)))
		return;

	if (File::CurrentApp && readTZDatabase(path.assign(File::CurrentApp.parent()).append("zoneinfo/").append(_name)))
		return;

#if !defined(_WIN32)
	path.assign("/etc/localtime");
	readTZDatabase(path);
#endif

}


Int32 Timezone::localOffset(const Date& date,UInt32 clock,bool& isDST) {
	isDST = false;

	if (date.year() <= 1916) {
		// no daylight before 30 april 1916
		if (date.year()<1916 || date.month()<4) 
			return _offset;
		if (date.month() == 4) {
			if (date.day() < 30)
				return _offset;
		}
	}

	if (_LocalTransitions.empty()) // no transitions, use default rule
		return localOffsetUsingRules(date, clock, isDST);


	Int64 time(date.time()+date.offset());

	auto it(_LocalTransitions.lower_bound(time)),end(_LocalTransitions.end());
	if (it == _LocalTransitions.end()) // if it's the last transitions, use default rules
		return localOffsetUsingRules(date, clock, isDST);
	if (it == _LocalTransitions.begin())
		return _offset; // before every transition, so no daylight (in the past, no daylight saving time!)
	if (it->first != time)
		--it;
	isDST = it->second.isDST;
	return it->second.offset;
}

Int32 Timezone::localOffset(Int64 time, TimeType& timeType) {
	timeType = STANDARD;

	if (time<-1693785600) // no daylight before the 30 april 1916
		return _offset;

	if (_Transitions.empty()) { // no transitions, use default rule
		timeType = CHECK_DST_WITH_RULES;
		return _offset;
	}

	auto it(_Transitions.lower_bound(time)),end(_Transitions.end());
	if (it == _Transitions.end()) { // if it's the last transitions, use default rules
		timeType = CHECK_DST_WITH_RULES;
		return _offset;
	}
	if (it == _Transitions.begin())
		return _offset; // before every transition, so no daylight (in the past, no daylight saving time!)
	if (it->first != time)
		--it;
	if (it->second.isDST)
		timeType = DST;
	return it->second.offset;
}

Int32 Timezone::localOffsetUsingRules(const Date& date,UInt32 clock,bool& isDST) {
	if (!_StartDST)
		return _offset;  // no daylight

	// no transitions, use default rule
	if (date.month() < _StartDST.month || (_EndDST && date.month() > _EndDST.month))
		return _offset; // no daylight

	if (date.month() == _StartDST.month)
		isDST = !isBeforeTransition(date,clock, _StartDST);
	else if (_EndDST && date.month() == _EndDST.month)
		isDST = isBeforeTransition(date, clock, _EndDST);
	else
		isDST = true;
	return isDST ? _dstOffset : _offset;
}
	
bool Timezone::isBeforeTransition(const Date& date,UInt32 clock,const TransitionRule& rule) {
	// Consider that we are on the same month, and rule!=NIL! (caller checking)

	if (rule.type == TransitionRule::ABSOLUTE) {
		if (date.yearDay()<rule.day)
			return true;
	} else if (rule.type == TransitionRule::JULIAN) {
		UInt16 day(date.yearDay());
		if (date.month()>2 && Date::IsLeapYear(date.year()))
			--day;
		if (day<rule.day)
			return true;
	} else {
		//RELATIVE
		UInt8 days = (date.day() + 6 - date.weekDay());
		UInt8 week(days/7);
		if ((days%7) > 0 && week<5)
			++week;
		// before week?
		if (week < rule.week)
			return true;
		// before day?
		if (date.weekDay() < rule.day)
			return true;
	}

	// before clock?
	return clock<rule.clock;
}

Int32 Timezone::Offset(const char* code ,bool& isDST) {
	if (String::ICompare(code,"Z")==0 || String::ICompare(code,"GMT")==0 || String::ICompare(code,"UTC")==0) {
		isDST = false;
		return Date::GMT;
	}
	auto it = Timezones.find(code);
	if (it == Timezones.end())
		return Date::GMT;
	isDST = it->second.second;
	return it->second.first;
}


} // namespace Mona
