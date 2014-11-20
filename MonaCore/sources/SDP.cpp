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

#include "Mona/SDP.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

void SDP::clearMedias() {
	map<string,SDPMedia*>::iterator it;
	for(it=_medias.begin();it!=_medias.end();++it)
		delete it->second;
	_medias.clear();
}

bool SDP::build(Exception& ex, const char* text) {

	SDPMedia* pMedia = NULL;

	String::ForEach forEach([this,&pMedia,&ex](UInt32 index,const char* line){
		if(strlen(line)==0 || line[1] != '=') {
			ex.set(Exception::FORMATTING, "SDP line ",line," malformed, second byte isn't an equal sign");
			return false;
		}
	
		UInt8 type = line[0];
		line = String::TrimLeft(line);

		// RFC 4566
		switch(type) {
			case 'v': // v=  (protocol version)
				version = String::ToNumber<UInt32>(ex, line);
				break;
			case 'o': { // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
				String::ForEach forEach([&ex,this](UInt32 index, const char* value) {
					switch (index) {
						case 0:
							user.assign(value);
							break;
						case 1:
							sessionId = String::ToNumber<UInt32>(ex, value);
							break;
						case 2:
							sessionVersion = String::ToNumber<UInt32>(ex, value);
							break;
						case 4:
							unicastAddress.set(IPAddress::Wildcard(String::ICompare(value,"IP6")==0 ? IPAddress::IPv6 : IPAddress::IPv4));
							break;
						case 5:
							unicastAddress.set(ex,value,unicastAddress.family());
							break;
					}
					return !ex;
				});
				if(String::Split(line, " ", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM)<6)
					ex.set(Exception::PROTOCOL, "SDP o line required 6 values (<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>)");
				break;
			}
			case 's': // s=  (session name)
				sessionName = line;
				break;

			/// Optional lines
			case 'i': // i=* (session information)
				sessionInfos = line;
				break;
			case 'u': // u=* (URI of description)
				uri = line;
				break;
			case 'e': // e=* (email address)
				email = line;
				break;
			case 'p': // p=* (phone number)
				phone = line;
				break;
			case 'c': { // c=* (connection information -- not required if included in all media)
				IPAddress defaultAddress;  // TODO defaultAddress is useless for what?
				String::ForEach forEach([&ex,&defaultAddress, this](UInt32 index, const char* value) {
					switch (index) {
						case 1:
							defaultAddress.set(IPAddress::Wildcard(String::ICompare(value,"IP6")==0 ? IPAddress::IPv6 : IPAddress::IPv4));
							break;
						case 2:
							defaultAddress.set(ex,value,defaultAddress.family());
							break;
					}
					return !ex;
				});
				if(String::Split(line, " ", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM)<3)
					ex.set(Exception::PROTOCOL, "SDP c line required 3 values");
				break;
			}
			case 'b': // b=* (zero or more bandwidth information lines)
				// TODO useless?
				break;
				 
				// One or more time descriptions ("t=" and "r=" lines; see below)
				// t=  (time the session is active)
				// r=* (zero or more repeat times)
			case 't':
				// TODO useless?
				break;
			case 'r':
				// TODO useless?
				break;
			case 'z': // z=* (time zone adjustments)
				// TODO useless?
				break;

			case 'k': // k=* (encryption key)
				encryptKey = line;
				break;
			case 'm': { // m=<name> <port> <proto> <fmt>
				string name;
				UInt16 port;
				String::ForEach forEach([&ex, &pMedia, &name, &port, this](UInt32 index, const char* value) {
					switch (index) {
						case 0:
							name = value;
							break;
						case 1:
							port = String::ToNumber<UInt16>(ex, value);
							break;
						case 2:
							pMedia = addMedia(name, port, value);
							break;
					}
					return !ex;
				});
				if(String::Split(line, " ", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM)<4)
					ex.set(Exception::PROTOCOL, "SDP m line required 4 values (<name> <port> <proto> <fmt>)");
				break;
			}
			case 'a': { // a=* (zero or more session attribute lines)

				vector<string>* pFields = NULL;
				bool	isMsId = false;
				string   fingerHash;

				// TODO SDPSource* pSource = NULL;
				// TODO list<UInt32>* pSourceGroupe = NULL;


				String::ForEach forEach([&ex, &pMedia, &pFields, &isMsId, &fingerHash, this](UInt32 index, const char* value) {

					// RFC 5576
					if(pMedia) {
						/* TODO
						if(pSourceGroupe)
							pSourceGroupe->push_back(NumberParser::parseUnsigned(field));
						else if(pSource) {
							if(isMsId)
								pSource->msData = field;
							else if(key=="cname") // cname:<value>
								pSource->cname = value;
							else if(key=="msid") {// draft-alvestrand-mmusic-msid-00
								isMsId = true;
								pSource->msId = value;
							} else if(key=="mslabel") // draft-alvestrand-rtcweb-mid-01
								pSource->msLabel = value;
							else if(key=="label") // draft-alvestrand-rtcweb-mid-01
								pSource->label = value;
							else
								WARN("Media source attribute ",key," unknown");
						} else if(key=="ssrc") // a=ssrc:<ssrc-id>
							pSource = &sources[NumberParser::parseUnsigned(key)];// a=ssrc:<ssrc-id> <attribute>
						else if(key=="rtcp-mux")
							pMedia->rtcpMux = true;
						else if(key=="mid")  // RFC 3388, a=mid:<token>
							pMedia->mid = value;
						else if(key=="ssrc-group")  // a=ssrc-group:<semantics>
							pSourceGroupe = &sourceGroupes[value];
						else if(key="crypto") { // a=crypto:<tag> <crypto-suite> <key-params> [<session-params>]
							StringTokenizer values(value," ",StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
							poco_assert(values.size()>2)
							pMedia->cryptoTag = 
						} else 
							WARN("Media attribute ",key," unknown");*/
						ERROR("TODO")
					} else if(pFields)
						pFields->emplace_back(value);
					else if(!fingerHash.empty())
						int i=0; // TODO finger = talk_base::SSLFingerprint::CreateFromRfc4572(fingerHash, field);
					else {
						const char* key(value);
						const char* value(String::Empty.c_str());
						const char* colon(strchr(value,':'));
						if (colon) {
							*(char*)colon = 0;
							value = colon + 1;
						}

						if (String::ICompare(key,"group")==0) // RFC 5888 and draft-holmberg-mmusic-sdp-bundle-negotiation-00
							pFields = &groups[value];
						else if (String::ICompare(key,"ice-ufrag")==0)
							iceUFrag = value;
						else if (String::ICompare(key,"ice-pwd")==0)
							icePwd = value;
						else if (String::ICompare(key,"ice-options")==0) {
							pFields = &iceOptions;
							iceOptions.emplace_back(value);
						} else if (String::ICompare(key,"fingerprint")==0) // fingerprint:<hash> <algo>
							fingerHash = value; // fingerprint:<hash>
						else if (String::ICompare(key,"msid-semantic")==0)
							supportMsId = String::ICompare(value ,"VMS")==0;
						else if (String::ICompare(key,"extmap")==0) // RFC 5285 a=extmap:<value>["/"<direction>] <URI> <extensionattributes>
							pFields = &extensions[value];
						else
							WARN("SDP attribute ", key, " unknown");
					}

					return true;
				});

				String::Split(line, " ", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
				break;
			} // case 'a':

		} // switch(type)

		if (ex) {
			ex.set(Exception::PROTOCOL, "SDP '",type,"' value ",line," malformed, ",ex.error());
			return false;
		}
		return true;
	});

	return String::Split(text, "\r\n", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM)!=string::npos;
}

void SDP::build() {
	// TODO
}


void SDP::SendNewCandidate(Writer& writer,SDPCandidate& candidate) {
	DataWriter& packet = writer.writeInvocation("__ice");
	packet.beginObject();
	packet.writeStringProperty("candidate",candidate.candidate);
	// TODO add? packet.writeStringProperty("sdpMid",candidate.mid);
	packet.writeNumberProperty("sdpMLineIndex",(double)candidate.mLineIndex);
	packet.endObject();
}

} // namespace Mona
