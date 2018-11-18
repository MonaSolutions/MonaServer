#pragma once
#include <vector>
#include "Mona/AMF.h"
#include <set>
#include "AMFObject.h"
namespace Mona
{
	

struct DirtyInfo {
	AMF::SharedObjectType type;
	std::string propertyName;
	UInt8*  id;
	DirtyInfo(AMF::SharedObjectType type, const std::string& propertyName, UInt8*  id = 0) : type(type), propertyName(propertyName), id(id)
	{

	}
};
typedef std::vector<DirtyInfo> Dirtyness;
class Room;
struct SharedObject {
	std::string name;
	uint32_t version = 0;
	bool versionIncremented = false;
	bool persistence;
	AMFObject::AMFMap payloads;
	std::set<UInt8*> clients;
	//map<uint32_t, Dirtyness> dirtyProps;
	Dirtyness broadcastInfo;
	std::set<UInt8*> initInfo;
	Room* room;

	SharedObject(const std::string& name, bool persistence, Room* room) :name(name), persistence(persistence), room(room) {
		//todo persistence
	}
	PacketWriter& writeHeader(PacketWriter& writer) const
	{
		writer.write16(name.length()).write(name.c_str(), name.length()).write32(version).write32(persistence ? 2 : 0).write32(0);
		return writer;
	}
	void onClientConnect(UInt8* id) {
		clients.insert(id);
		initInfo.insert(id);
	}
	void onClientDisconnect(UInt8* id) {
		clients.erase(id);
		initInfo.erase(id);
	}
	AMFObject& operator [](const std::string& key) {
		return payloads[key];
	}
	void set(const std::string& key, AMFObject&& value, UInt8* id = nullptr);
	void unSet(const std::string& key);
	void track();
};
}
