#pragma once
#include "Mona/Mona.h"
#include "Mona/Publication.h"
#include "Mona/Publications.h"
#include <Mona/Entities.h>
#include "Mona/Peer.h"
#include "Mona/Handler.h"
#include "SharedObject.h"
namespace Mona
{
	class Room: public virtual Object
	{
		std::map<std::string, Publication>				_publications;
		Publication*			publish(Exception& ex, const std::string& name, Publication::Type type, Peer* pPeer);
		void					unpublish(const std::string& name, Peer* pPeer);

		std::string&			publicationName(std::string& name, std::string& query);
		std::map<std::string, SharedObject*> _sos;
	public:
		std::string				name;
		std::string				path;
		Publications			publications;
		Entities<Client>		clients;
		Handler*				handler;
		Publication*			publish(Exception& ex, const std::string& name, Publication::Type type) { return publish(ex, name, type, nullptr); }
		void					unpublish(const std::string& name) { unpublish(name, nullptr); }
		Publication*			publish(Exception& ex, Peer& peer, const std::string& name, Publication::Type type) { return publish(ex, name, type,&peer); }
		void					unpublish(Peer& peer, const std::string& name) { unpublish(name, &peer); }
		Listener*				subscribe(Exception& ex, Peer& peer, std::string& name, Writer& writer);
		Listener*				subscribe(Exception& ex, Peer& peer, const std::string& name, Writer& writer, const char* queryParams = nullptr);
		void					unsubscribe(Peer& peer, const std::string& name);
		Room(Handler* handler,const std::string& name,const std::string& path);
		~Room();
		SharedObject*			getSO(const std::string& name,bool persistence);
		void onClientDisconnect(Client& client);
	};
}
