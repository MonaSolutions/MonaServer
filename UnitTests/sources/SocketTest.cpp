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

#include "Test.h"
#include "Mona/TCPClient.h"
#include "Mona/TCPServer.h"
#include "Mona/UDPSocket.h"
#include "Mona/Logs.h"
#include <set>

using namespace std;
using namespace Mona;

class Mutex {
public:
	Mutex(bool factice = false) : _factice(factice) {}
	void lock() { if (_factice) return;  _mutex.lock(); }
	void unlock() { if (_factice) return;  _mutex.unlock(); }

private:
	bool		_factice;
	std::mutex	_mutex;
};

class TCPEchoClient : public TCPClient {
public:
	TCPEchoClient(const SocketManager& manager,bool parallel) : _unknown(false),testUnknown(false),TCPClient(manager),_mutex(!parallel),parallel(parallel) {}

	const bool parallel;
	bool testUnknown;

	void onError(const std::string& error) {
		if (!testUnknown)
			FATAL_ERROR("TCPEchoClient, ", error)
	}

	bool join() {
		if (!parallel) {
			if (testUnknown)
				return _unknown;
			lock_guard<Mutex> lock(_mutex);
			return _datas.empty();
		}
		while(_event.wait(20000)) {
			lock_guard<Mutex> lock(_mutex);
			if (testUnknown)
				return _unknown;
			if (_datas.empty())
				return true;
		}
		return false;
	}

	bool echo(Exception& ex,const UInt8* data,UInt32 size) {
		{
			lock_guard<Mutex> lock(_mutex);
			_datas.emplace_back(size);
			memcpy(_datas.back().data(), data, size);
		}
		return send(ex, data, size);
	}

private:
	UInt32 onReception(PoolBuffer& pBuffer) {
		lock_guard<Mutex> lock(_mutex);
		if (pBuffer->size() < _datas.front().size())
			return pBuffer->size(); // wait more data
		UInt32 consumed(_datas.front().size());
		CHECK(memcmp(_datas.front().data(), pBuffer->data(), consumed) == 0);
		_datas.pop_front();
		if (parallel)
			_event.set();
		return pBuffer->size()-consumed; // rest
	}

	void onDisconnection(){
		if (!testUnknown)
			return;
		_unknown=true;
		_event.set();
	}
	
	deque<vector<UInt8>> _datas;
	Event				_event;
	Mutex				_mutex;
	atomic<bool>		_unknown;
};



class TCPEchoServer : public TCPServer {
public:
	TCPEchoServer(const SocketManager& manager) : TCPServer(manager) {
	}

	virtual ~TCPEchoServer() {
		for (Client* pClient : _clients)
			delete pClient;
	}

private:
	class Client : public TCPClient {
	public:
		Client(const SocketAddress& peerAddress,const SocketManager& manager) : TCPClient(peerAddress, manager) {}

		void onError(const std::string& error) {
			FATAL_ERROR("Client, ",error);
		}
	private:
		UInt32 onReception(PoolBuffer& pBuffer) {
			Exception ex;
			CHECK(send(ex, pBuffer->data(), pBuffer->size()) && !ex);
			return 0;
		}

		void onDisconnection() {}

	};

	void onError(const string& error) {
		FATAL_ERROR("TCPServer, ",error);
	}

	void onConnectionRequest(Exception& ex) {
		Client* pClient = acceptClient<Client>(ex);
		if (!pClient)
			ERROR("NO CLIENT");
		CHECK(pClient);	
		_clients.emplace(pClient);
	}

	std::set<Client*> _clients;

};


class UDPEchoClient : public UDPSocket {
public:
	UDPEchoClient(const SocketManager& manager,bool parallel) : UDPSocket(manager) ,_mutex(!parallel),parallel(parallel) {}
	
	const bool parallel;

	void onError(const std::string& error) {
		FATAL_ERROR("UDPEchoClient, ",error);
	}

	bool join() {
		if (!parallel) {
			lock_guard<Mutex> lock(_mutex);
			return _datas.empty();
		}
		while(_event.wait(20000)) {
			lock_guard<Mutex> lock(_mutex);
			if (_datas.empty())
				return true;
		}
		return _datas.empty();
	}

	bool connect(Exception& ex, const SocketAddress& address) {
		bool result = UDPSocket::connect(ex, address);
		lock_guard<Mutex> lock(_mutex);
		_address.set(address);
		return result;
	}

	bool echo(Exception& ex,const UInt8* data,UInt32 size) {
		{
			lock_guard<Mutex> lock(_mutex);
			_datas.emplace_back(size);
			memcpy(_datas.back().data(), data, size);
		}
		return send(ex, data, size);
	}

private:
	void onReception(PoolBuffer& pBuffer,const SocketAddress& address) {
		lock_guard<Mutex> lock(_mutex);
		CHECK(_address == address);
		CHECK(_datas.front().size() == pBuffer->size() && memcmp(_datas.front().data(), pBuffer->data(), pBuffer->size()) == 0);
		_datas.pop_front();
		if (parallel)
			_event.set();
	}

	Mutex						_mutex;
	std::deque<vector<UInt8>>   _datas;
	Event					    _event;
	SocketAddress				_address;
};



class UDPEchoServer : public UDPSocket {
public:
	UDPEchoServer(const SocketManager& manager) : UDPSocket(manager) {}

private:
	
	void onError(const string& error) {
		FATAL_ERROR("UDPEchoServer, ",error);
	}

	void onReception(PoolBuffer& pBuffer,const SocketAddress& address) {
		Exception ex;
		CHECK(send(ex,pBuffer->data(), pBuffer->size(), address) && !ex);
	}
};



static Event TaskEvent;
class TaskHandlerSockets : public TaskHandler {
public:
	TaskHandlerSockets() {
		start();
	}

	template<typename SocketType>
	bool join(SocketType& socket) {
		if (socket.parallel)
			return socket.join();
		bool joined(false);
		while (!(joined=socket.join()) && TaskEvent.wait(20000)) {
			Exception ex;
			giveHandle(ex);
			CHECK(!ex);
		}
		return joined;
	}

private:
	void requestHandle() {
		TaskEvent.set();
	}
};

static string					Short0Data(1024, '\0');
static string					Long0Data(9999999, '\0');
static PoolThreads				Threads;
static PoolBuffers				Buffers;
static SocketManager			ParallelSockets(Buffers,Threads);
static TaskHandlerSockets		TaskSockets;
static SocketManager			Sockets(TaskSockets,Buffers,Threads);


void TCPTest(SocketManager& sockets) {
	Exception ex;

	SocketAddress host(IPAddress::Wildcard(),62435);
	TCPEchoServer server(sockets);
	CHECK(server.start(ex, host) && !ex);
	server.stop();
	CHECK(server.start(ex, host) && !ex);

	TCPEchoClient client(sockets, &sockets == &ParallelSockets);
	SocketAddress target(IPAddress::Loopback(),host.port());
	CHECK(client.connect(ex, target) && !ex && client.connected());

	CHECK(client.echo(ex,EXPAND_DATA_SIZE("hi mathieu and thomas")) && !ex);
	CHECK(client.echo(ex,(const UInt8*)Long0Data.c_str(),Long0Data.size()) && !ex);
	CHECK(TaskSockets.join(client));

	client.disconnect();
	CHECK(!client.connected());

	// Test an unknown connection
	SocketAddress unknown(IPAddress::Loopback(),62434);
	client.testUnknown = true;
	if (client.connect(ex, unknown)) {
		if (!client.parallel) {
			CHECK(!ex && client.connected());
			client.address();
			client.peerAddress();
			if (Util::Random<UInt8>()%2)
				CHECK(client.send(ex,EXPAND_DATA_SIZE("salut")) && !ex)
			CHECK(client.connected());
		}
		CHECK(TaskSockets.join(client));
	}
	CHECK(!client.connected());
}

void UDPTest(SocketManager& sockets) {
	Exception ex;

	SocketAddress	 host(IPAddress::Wildcard(),62435);
	UDPEchoServer    server(sockets);
	CHECK(server.bind(ex, host) && !ex);
	server.close();
	CHECK(server.bind(ex, host) && !ex);

	UDPEchoClient    client(sockets,&sockets==&ParallelSockets);
	SocketAddress	 target(IPAddress::Loopback(),host.port());
	CHECK(client.connect(ex, target) && !ex);

	CHECK(client.echo(ex,EXPAND_DATA_SIZE("hi mathieu and thomas")) && !ex);
	CHECK(client.echo(ex,(const UInt8*)Short0Data.c_str(),Short0Data.size()) && !ex);
	CHECK(TaskSockets.join(client));
}

ADD_TEST(SocketTest, StartSockets) {
	Exception ex;
	CHECK(Sockets.start(ex) && !ex && Sockets.running());
	CHECK(ParallelSockets.start(ex) && !ex && ParallelSockets.running());
}

ADD_TEST(SocketTest, TCPSocket) {
	TCPTest(Sockets);
}

ADD_TEST(SocketTest, TCPParallelSocket) {
	TCPTest(ParallelSockets);
}

ADD_TEST(SocketTest, UDPSocket) {
	UDPTest(Sockets);
}

ADD_TEST(SocketTest, UDPParallelSocket) {
	UDPTest(ParallelSockets);
}

ADD_TEST(SocketTest, StopSockets) {
	Sockets.stop();
	CHECK(!Sockets.running());
	ParallelSockets.stop();
	CHECK(!ParallelSockets.running());
}