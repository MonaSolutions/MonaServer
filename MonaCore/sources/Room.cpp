#include <Room.h>
#include <Mona/Logs.h>
using namespace std;
namespace Mona
{
	Room::Room(Handler* handler, const std::string& name, const std::string& path):name(name), path(path), publications(_publications),clients(),handler(handler)
	{
		INFO("Room Created:", path);
	}
	Room::~Room()
	{
		
	}

	SharedObject* Room::getSO(const std::string& name, bool persistent)
	{
		auto& so = _sos[name];
		if (!so)so = new SharedObject(name, persistent, this);
		return so;
	}
	void Room::onClientDisconnect(Client& client)
	{
		FOR_ITEM(_sos,so)
		{
			so.second->onClientDisconnect(client.id);
		}
		clients.remove(client);
	}
	Publication* Room::publish(Exception& ex, const string& name, Publication::Type type, Peer* pPeer) {
		MAP_FIND_OR_EMPLACE(_publications, it, name, name, handler->poolBuffers);
		Publication& publication(it->second);

		if (publication.running()) { // is already publishing
			ex.set(Exception::SOFTWARE, name, " is already publishing");
			return nullptr;
		}

		if (pPeer ? pPeer->onPublish(ex, publication):handler->onPublish(ex,publication,nullptr)) {
			string mediaFolder("./www" + pPeer->path + "/media/");
			if (!FileSystem::Exists(mediaFolder))FileSystem::CreateDirectory(mediaFolder);
			publication.start(pPeer && type != Publication::Type::LIVE ? mediaFolder + name + ".flv" : "", type == Publication::Type::APPEND);
			return &publication;
		}

		if (!ex)
			ex.set(Exception::APPLICATION, "Not allowed to publish ", name);
		if (publication.listeners.count() == 0)
			_publications.erase(it);
		return nullptr;
	}


	void Room::unpublish(const string& name, Peer* pPeer) {

		auto it = _publications.find(name);
		if (it == _publications.end()) {
			DEBUG("The publication '", name, "' doesn't exist, unpublish useless");
			return;
		}
		Publication& publication(it->second);
		if (publication.running()) {
			if(pPeer)pPeer->onUnpublish(publication);
			else handler->onUnpublish(publication,nullptr);
			publication.stop();
		}

		if (publication.listeners.count() == 0)
			_publications.erase(it);
	}

	Listener* Room::subscribe(Exception& ex, Peer& peer, string& name, Writer& writer) {
		string query;
		Listener* pListener(subscribe(ex, peer, (const string&)publicationName(name, query), writer, query.c_str()));
		if (pListener)
			Util::UnpackQuery(query, *pListener);
		return pListener;
	}

	Listener* Room::subscribe(Exception& ex, Peer& peer, const string& name, Writer& writer, const char* queryParams) {
		MAP_FIND_OR_EMPLACE(_publications, it, name, name, handler->poolBuffers);
		Publication& publication(it->second);
		Listener* pListener = publication.addListener(ex, peer, writer, queryParams);
		if (!pListener) {
			if (!publication.running() && publication.listeners.count() == 0)
				_publications.erase(it);
		}
		return pListener;
	}

	void Room::unsubscribe(Peer& peer, const string& name) {
		auto it = _publications.find(name);
		if (it == _publications.end()) {
			DEBUG("The publication '", name, "' doesn't exists, unsubscribe useless");
			return;
		}
		Publication& publication(it->second);
		publication.removeListener(peer);
		if (!publication.running() && publication.listeners.count() == 0)
			_publications.erase(it);
	}

	string& Room::publicationName(string& name, string& query) {
		size_t found(name.find('?'));
		if (found != string::npos) {
			query = (&name[found] + 1);
			name.assign(name, 0, found);
		}
		else
			query.clear();
		return name;
	}
}
