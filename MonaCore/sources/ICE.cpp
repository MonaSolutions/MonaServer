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

#include "Mona/ICE.h"
#include "Mona/RelayServer.h"
#include "Mona/Logs.h"
#include "Mona/String.h"

using namespace std;



namespace Mona {

ICE::ICE(const Peer& initiator,const Peer& remote,const RelayServer& relay) : _first(true),mediaIndex(0),_relay(relay),_type(INITIATOR),_publicHost(initiator.address.host().toString()),_serverRemoteHost(remote.serverAddress.host().toString()),_serverInitiatorHost(initiator.serverAddress.host().toString()),_initiator(initiator),_remote(remote) {
}

ICE::~ICE() {
}

void ICE::setCurrent(Peer& current) {
	_publicHost = current.address.host().toString();
	_type = current==_initiator?INITIATOR:REMOTE;
}

void ICE::reset() {
	_initiatorAddresses.clear();
	_remoteAddresses.clear();
	_type = INITIATOR;
	_time.update();
	_relayPorts.clear();
	mediaIndex=0;
	_first = true;
}

void ICE::fromSDPLine(const string& line,SDPCandidate& publicCandidate) {
	list<SDPCandidate> relayCurrentCandidates;
	list<SDPCandidate> relayRemoteCandidates;
	fromSDPLine(line,publicCandidate,relayCurrentCandidates,relayRemoteCandidates,false);
}

void ICE::fromSDPLine(const string& line,SDPCandidate& publicCandidate,list<SDPCandidate>& relayCurrentCandidates,list<SDPCandidate>& relayRemoteCandidates,bool relayed) {
	relayCurrentCandidates.clear();
	relayRemoteCandidates.clear();
	publicCandidate.candidate.clear();

	if(line.compare(0,2,"v=")==0) {
		if(_type==INITIATOR)
			reset();
		else {
			mediaIndex=0;
			_first=true;
		}
	} else if(line.compare(0,2,"m=")==0) {
		if(_first)
			_first=false;
		else
			++mediaIndex;
	} else if(line.compare(0,12,"a=candidate:")==0)
		fromSDPCandidate(line,publicCandidate,relayCurrentCandidates,relayRemoteCandidates,relayed);
}

void ICE::fromSDPCandidate(const string& candidate,SDPCandidate& publicCandidate,list<SDPCandidate>& relayCurrentCandidates,list<SDPCandidate>& relayRemoteCandidates,bool relayed) {
	
	const char* end = NULL;
	do {
		if(end)
			((string&)candidate).resize(candidate.size()-2);
		if(candidate.size()<2)
			return;
		end = &candidate[candidate.size()-2];
	} while(memcmp(end,"\\r",2)==0 || memcmp(end,"\\n",2)==0);

	vector<string> fields;
	String::Split(candidate, " ", fields, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	UInt8 index=0;
	string port;
	const string* pHost=NULL;
	bool isTCP=false;
	string& content=publicCandidate.candidate;
	UInt32 compoment=0;
	for(string& st : fields) {
		if(index>0)
			content.append(" ");
		if(++index==2) {
			Exception ex;
			compoment = String::ToNumber<UInt32>(ex, st);
			content += st;
		} else if (index == 3 && String::ICompare(st, "tcp") == 0) {
			isTCP = true;
			content += st;
		} else if(index==4)
			content += "1845501695";
		else if(index==5) {
			content += _publicHost; // address
			pHost = &st;
		} else if(index==6) {
			content += st;
			port = st; // port	
		} else if(index==8 && String::ICompare(st,"host")!=0) {
			content.clear();
			return;
		} else
			content += st;
	}
	if(!content.empty()) {
		//content.clear();
		content.append("\\r\\n");
		publicCandidate.mLineIndex = mediaIndex;
	}

	DEBUG(_type==INITIATOR ? "Offer" : "Anwser"," candidate ",mediaIndex,"[",compoment,"] ",isTCP ? "TCP" : "UDP"," ",pHost ? pHost->c_str() : "?",":",port)

	if(relayed && !isTCP && !port.empty()) {
		// informations usefull just for relay line!
		
		Exception ex;
		int valport = String::ToNumber<int>(ex, port);
		if (ex) {
			WARN("Invalid candidate address, ",ex.error());
			return ;
		}

		SocketAddress address;
		address.set(ex,_publicHost, valport);
		if (ex) {
			WARN("Invalid candidate address")
			return;
		}
			
		set<SocketAddress>& currentAddresses = _type==INITIATOR ? _initiatorAddresses[mediaIndex][compoment] : _remoteAddresses[mediaIndex][compoment];
		set<SocketAddress>& remoteAddresses = _type==INITIATOR ?  _remoteAddresses[mediaIndex][compoment] :_initiatorAddresses[mediaIndex][compoment];

		if(currentAddresses.insert(address).second) {
			set<SocketAddress>::const_iterator it;
			for(it=remoteAddresses.begin();it!=remoteAddresses.end();++it) {
				UInt16 port = _relay.add(_initiator,*it,_remote,address);
				if(port==0)
					continue;
				if(_relayPorts[mediaIndex][compoment].insert(port).second) {
					relayCurrentCandidates.resize(relayCurrentCandidates.size()+1);
					SDPCandidate& currentCandidate = relayCurrentCandidates.back();
					String::Format(currentCandidate.candidate, "a=candidate:7 ",compoment," udp 1 ",_type==INITIATOR ? _serverInitiatorHost : _serverRemoteHost," ",port," typ host\\r\\n");
					currentCandidate.mLineIndex = mediaIndex;

					relayRemoteCandidates.resize(relayRemoteCandidates.size()+1);
					SDPCandidate& remoteCandidate = relayRemoteCandidates.back();
					String::Format(remoteCandidate.candidate, "a=candidate:7 ",compoment," udp 1 ",_type==INITIATOR ? _serverRemoteHost : _serverInitiatorHost," ",port," typ host\\r\\n");
					remoteCandidate.mLineIndex = mediaIndex;
				}
			}	
		}
	}
}


bool ICE::ProcessSDPPacket(DataReader& packet,Peer& current,Writer& currentWriter,Peer& remote,Writer& remoteWriter) {
	if(packet.followingType()!=DataReader::STRING) {
		packet.reset();
		return false;
	}
	string name;
	if (packet.readString(name) != "__ice") {
		packet.reset();
		return false;
	}

	SDPCandidate		candidate;
	list<string>		newLines;
	list<SDPCandidate>	relayCurrentCandidates,relayRemoteCandidates;
	DataWriter& writer = remoteWriter.writeInvocation(name);
	ICE& ice = current.ice(remote);

	DataReader::Type type;
	while((type = packet.followingType())!=DataReader::END) {
		if(type!=DataReader::OBJECT) {
			packet.read(writer,1);
			continue;
		}
		
		writer.beginObject();
		string prop;
		bool external; // TODO what's happen when external==true (ArrayList in AMF for example)
		packet.readObject(prop,external);
		while((type = packet.readItem(prop))!=DataReader::END) {
			if(!prop.empty())
				writer.writePropertyName(prop);

			if(prop=="sdpMid" && type==DataReader::STRING) {
				writer.writeString(packet.readString(candidate.mid));
				continue;
			} else if(prop=="sdpMLineIndex" && type==DataReader::NUMBER) {
				candidate.mLineIndex = (UInt32)packet.readNumber();
				writer.writeNumber(candidate.mLineIndex);
				continue;
			} else if(prop=="candidate") {
				packet.readString(candidate.candidate);
				/* To test without host candidates
				string::size_type found = candidate.candidate.find("192.168.1.11");
				if(found==string::npos)
					found = candidate.candidate.find("192.168.56.1");
				if(found!=string::npos)
					writer.writeString(candidate.candidate.replace(found,12,"192.168.1.17"));
				else*/
				writer.writeString(candidate.candidate);
				continue;
			} else if(type!=DataReader::STRING || prop!="sdp") {
				packet.read(writer,1);
				continue;
			}

			// Just in SDP case!
			string content;
			packet.readString(content);
			string::size_type pos,lastPos = 0;
			do {
				pos = content.find("\\r\\n",lastPos);
				if(pos == string::npos)
					 pos = content.length();
				if(pos != lastPos) {
					const string line(content.data()+lastPos,pos-lastPos);
					ice.fromSDPLine(line,candidate,relayCurrentCandidates,relayRemoteCandidates);
					/* To test without host candidates
					if(line.compare(0,12,"a=candidate:")==0) {
						content.erase(lastPos,pos-lastPos+4);
						pos -= (line.size()+4);
					}*/
					if(!candidate.candidate.empty()) {
						content.insert(lastPos,candidate.candidate);
						pos += candidate.candidate.size();
						candidate.candidate.clear();
					}
					list<SDPCandidate>::iterator it;
					for(it=relayCurrentCandidates.begin();it!=relayCurrentCandidates.end();++it) {
						content.insert(lastPos,it->candidate);
						pos += it->candidate.size();
					}
					for(it=relayRemoteCandidates.begin();it!=relayRemoteCandidates.end();++it)
						SDP::SendNewCandidate(currentWriter,*it);
				}
				lastPos = pos + 4;
			} while(lastPos<content.length());
			writer.writeString(content);
		}
		writer.endObject();
	}

	if(!candidate.candidate.empty()) {
		ice.mediaIndex = candidate.mLineIndex;
		SDPCandidate publicCandidate;
		ice.fromSDPLine(candidate.candidate,publicCandidate,relayCurrentCandidates,relayRemoteCandidates);
		if(!publicCandidate.candidate.empty())
			SDP::SendNewCandidate(currentWriter,publicCandidate);
		list<SDPCandidate>::iterator it;
		for(it=relayCurrentCandidates.begin();it!=relayCurrentCandidates.end();++it)
			SDP::SendNewCandidate(currentWriter,*it);
		for(it=relayRemoteCandidates.begin();it!=relayRemoteCandidates.end();++it)
			SDP::SendNewCandidate(remoteWriter,*it);
	}

	return true;
}



} // namespace Mona
