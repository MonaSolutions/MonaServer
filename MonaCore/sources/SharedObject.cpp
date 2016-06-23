#include "SharedObject.h"
#include "Room.h"
namespace Mona
{
	void SharedObject::set(const std::string& key, AMFObject&& value, UInt8* id) {
		if (!versionIncremented) {
			version++;
			versionIncremented = true;
		}
		if MAP_NOTHAS1(payloads, key)
			payloads.emplace(key, move(value));
		else payloads[key] = move(value);
		/*FOR_ITEM(clients, client) {
		dirtyProps[client].push_back({ client ==id?AMF::SC_UPDATE_DATA_ACK :AMF::SC_UPDATE_DATA,key});
		}*/
		broadcastInfo.emplace_back(AMF::SC_UPDATE_DATA, key, id);
	}
	void SharedObject::unSet(const std::string& key) {
		if (!versionIncremented) {
			version++;
			versionIncremented = true;
		}
		if MAP_HAS1(payloads, key)payloads.erase(key);

		/*FOR_ITEM(clients, client) {
		dirtyProps[client].push_back({AMF::SC_DELETE_DATA ,key });
		}*/
		broadcastInfo.emplace_back(AMF::SC_DELETE_DATA, key);
	}
	void SharedObject::track()
	{
		if (!initInfo.empty()) {
			FOR_ITEM(initInfo, clientId) {
				auto client = room->clients(clientId);
				client->SendInitSharedObjectMessage(*this);
			}
			broadcastInfo.clear();
			initInfo.clear();
		}else if (!broadcastInfo.empty()) {
			FOR_ITEM(broadcastInfo, i) {
				FOR_ITEM(clients, clientId) {
					auto client = room->clients(clientId);
					client->SendSharedObjectMessage(*this, i);
				}
			}
			broadcastInfo.clear();
		}
		versionIncremented = false;
	}
}