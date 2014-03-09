
.. image:: githubBlack.png
  :align: right
  :target: https://github.com/MonaSolutions/MonaServer

Scalability and load-balancing
###################################

Even if RTMFP is a P2P protocol, RTMFP has need to have a server end-point to negotiate the P2P connection between both clients. Then, RTMFP can be used just for its UDP based abilities, like real-time video/audio streaming, in a classical client-server way (without using P2P feature).
For these reasons, server load is always applied and central availability required.
Of course, one machine has always some hardware limitation (CPU/memory), and when load required becomes too important, a MonaServer instance can not be enough.

To solve this loading requirement, a full framework included in Monaerver allows to make communicate multiple MonaServer instances, to enlarge computing and receiving capacity. It allows to configure a multiple MonaServer environment, it offers detection of server connection and disconnection, exchange of data between them, redirection of clients between servers to deploy a load-balacing software system for example, and some well-thought features to synchronise client informations for the rendez-vous service and the NetGroup RTMFP options. Every communication between servers is done in a raw TCP way.

The main idea is simple: by default, **each instance is an independant server and share nothing with others, it's you who decides what are the resources that it has to share** between all the server instances.

This page intends to describe every features of this framework illustrated with some code samples and context usage. Of course, the `Server Application, API <./api.html>`_ page lists all these feature but without code samples or any utilization context.

Finally some piece of script code illustrates as uses it, to know how create a application server see `Server Application <./serveapp.html>`_ page.

.. contents:: Table of Contents

Configurations
***********************************

Firstly to make communicate many instances of MonaServer, you have to configure them. The three following configurations allows to make working the multiple servers mode:

- *publicAddress* to configure the public address server to make working the client redirections.
- *servers.port* to configure the port to receive incoming server connections.
- *servers.targets* to configure the addresses of remote MonaServer instances trying to join.

Here follows an illustration of one configuration with two servers:

.. TODO see if we can do a ppt
.. <center>![MonaServers](http://jazzmatazz.free.fr/Mona/MonaServers.PNG)</center>

.. warning:: Exchange between servers is done in a uncrypted TCP way, so to avoid an attack by the incoming port of B, its *servers.port* configured should be protected by a firewall to allow just a connection by an other server and nothing else.

A initializes here the connection to B (*server.targets* configured). A sees B as a target:

.. code-block:: lua

	-- Server application on A side
	function onServerConnection(server)
		if server.isTarget then
			NOTE("Target gotten : "..server.address.."("..server.publicAddress.." for clients)"
			-- displays "Target gotten : 192.168.0.2 (www.hostB.com for clients)"
		end
	end

B, who has a incoming port configured (1936), accepts the connection of A. B sees A as an initiator:

.. code-block:: lua

	-- Server application on B side
	function onServerConnection(server)
		NOTE(server.isTarget) -- displays "false"
	end

.. warning::  If server A and B configures each other as its target, the two TCP connections will be created, causing confusion in server exchange:

.. TODO see if we can do a ppt
.. <center>![MonaServers_DoubleConnection](http://jazzmatazz.free.fr/Mona/MonaServers_DoubleConnection.PNG)</center>

This configuration system allows to scale an existing system horizontaly without having to restart server already running. Indeed, the first server started can configure its incoming server port (*servers.port*) and no target, and a new server can come to extend the system in putting the address of the first server in its *servers.targets* configuration.

Of course, complex configurations are possible, with multiple server (and properties individual by server, see *Configurations* part of `Installation <./installation.html>`_ page):

.. code-block:: ini

	;MonaServer.ini
	publicAddress = www.myhost.com:1935
	[servers]
	targets = 192.168.0.2:1936?type=master;192.168.0.3:1936

.. code-block:: lua

	function onServerConnection(server)
		if server.type=="master" then -- true here just for 192.168.0.2:1936 server
			NOTE("Master server connected")
		end
	end
	function onServerDisconnection(server)
		if server.type=="master" then -- true here just for 192.168.0.2:1936 server
			NOTE("Master server disconnected")
		end
	end

.. warning::  The server applications which have the same path (*www/myGame* on server A and on server B) are synchronized but reloaded always just on connection client. It means that if you edit the file *www/myGame/main.lua* on the server A, it rebuilds the server A version on new connection client, and tries to rebuild the server B version too (of course reloading is effective just if the server B version has changed too). But if you edit the server B version and that clients are always connected by the server A intermediate, you have to edit the server A version too to get a refresh of the server B application on connection client.


Exchange data and resources
***********************************

To exchange data between servers you have to call the *server:send* method on sender side (see *server* object description on `Server Application, API <./api.html>`_) and you have to define RPC server functions as a member of server object on the receiver side: 

.. code-block:: lua

	function onServerConnection(server)
		-- RPC function declaration, to receive data from one other server
		function server:onHello(name)
			self.name = name
		end
		-- send my name to the incoming server (it will receive it on its "onHello" method)
		server:send("onHello","MonaServer A")
	end

	-- now you can find the name of each server everywhere
	for index,server in mona.servers:ipairs() do
		NOTE("Server '"..server.name.."' at address "..server.address)
	end

.. warning:: *self.name = name* in the function body of *onHello* creates on the *server* object a *name* value. Beware with this kind of thing on *server* object, it's shared with all other `Server Application <./serveapp.html>`_. If one other server application attachs too a *name* value to this *server* object, it will overload the previous assignment. A solution can be to prefix the property by the name of the current application.

The main goal of this exchange mechanism is to share resource wanted between all the server instances.
For example, if you use Mona to stream (by server bypass configuration, no P2P) to many subscribers, usually there are a small number of publishers and a very important number of subscribers. The server can support the publisher load, but could be saturated by the important number of listeners.
One solution in this model case is to scale horizontaly the system to share the subscribers load.

.. TODO see if we can do a ppt
.. <center>![MonaServers_Publication](http://jazzmatazz.free.fr/Mona/MonaServers_Publication.PNG)</center>

Here we have a configuration with three servers, but a fourth server could be added dynamically. The load-balacing system can be managed by a DNS way, but we have to share the publications between all three (or four) servers, otherwise one subscriber could not find one publication. Below following a complete `Server Application <./serveapp.html>`_ to share publications between all the servers.

.. code-block:: lua

	-- following server (horizontal scaling)
	_server = nil

	-- number of subscribers (listeners) for this server
	_subscribers = 0

	function onServerConnection(server)
		if server.isTarget then
			-- incoming server is a following server!
			if _server then error("following server already connected") end
			_server = server
			-- informs the following server about my publications
			for id,publication in mona.publications:pairs() do
				_server:send("publish",publication.name)
			end
		else
			-- incoming server is a previous server, we have to create RPC function to receive
			-- its publication informations
			server.publications = {}
			function server:publish(name)
				-- publication creation
				self.publications[name] = mona:publish(name)
			end
			function server:unpublish(name)
				-- publication suppression
				local publication = self.publications[name]
				if publication then publication:close() end
				self.publications[name] = nil
			end
			function server:video(name,time,packet)
				local publication = self.publications[name]
				-- give the video packet to our publication copy
				if publication then publication:pushVideoPacket(time,packet) end
			end
			function server:audio(name,time,packet)
				local publication = self.publications[name]
				-- give the audio packet to our publication copy
				if publication then publication:pushVideoPacket(time,packet) end
			end
			function server:data(name,time,packet)
				local publication = self.publications[name]
				-- give the data packet to our publication copy
				if publication then publication:pushVideoPacket(time,packet) end
			end
		end
	end

	function onServerDisconnection(server)
		if server.isTarget then
			-- disconnected server was a following server!
			_server = nil
			return
		end
		-- disconnected server was a previous server, close its publications
		for id,publication in server.publications do
			publication:close()
		end
	end

	function onPublish(client,publication)
		-- informs the following server about this publication
		if _server then _server:send("publish",publication.name) end
	end

	function onUnpublish(client,publication)
		-- informs the following server about this unpublication
		if _server then _server:send("unpublish",publication.name) end
	end

	function onSubscribe(client,listener)
		-- if a following server exist, and if this server has more than 400 subscribers
		-- redirect the client to the following server:
		-- I send an error with the redirection server address in its description
		if _server and _subscribers>=400 then error(_server.publicAddress) end
		_subscribers = _subscribers + 1
	end

	function onUnsubscribe(client,listener)
		_subscribers = _subscribers - 1
	end
	
	function onVideoPacket(client,publication,time,packet)
		if not _server then return end
		-- forward the video packet to the following server
		_server:send("video",publication.name,time,packet)
	end

	function onAudioPacket(client,publication,time,packet)
		if not _server then return end
		-- forward the audio packet to the following server
		_server:send("audio",publication.name,time,packet)
	end

	function onDataPacket(client,publication,name,packet)
		if not _server then return end
		-- forward the data packet to the following server
		_server:send("data",publication.name,time,packet)
	end

The line *if _server and _subscribers>=400 then error(_server.publicAddress) end* requires a specific client code to work, to redirect as wanted the new subscriber to the new server :

.. code-block:: as3

  function onStatusEvent(event:NetStatusEvent):void {
    switch(event.info.code) {
      case "NetStream.Play.Failed":
      _netConnection.close()
      _netConnection.connect(event.info.description); // error desciption contains the redirection server address
      break;
    }
  }


Load balancing and rendezvous service
******************************************

In a load-balacing solution, usually we opt for hardware solution with a DNS which returns an address ip rotated on a list of addresses. You can realize it in a software way in using the *onHandshake* event (see `Server Application, API <./api.html>`_ page for complete details on this event):

.. code-block:: lua

	-- index incremented to redirect client equally to each server
	index=0
	function onHandshake(address,path,properties,attempts)
		index=index+1
		if index > mona.servers.count then index=1 end -- not exceed the number of server available
		return mona.servers(index) -- load-balacing system!
	end

Here the server doesn't accept any connection client, it redirects the cleint in handshake performing. There is no real benefits comparing with a hardware solution.
An other possibility is of returning many server addresses to benefit of parallel connection behavior of RTMFP protocol.

.. code-block:: lua

	function onHandshake(address,path,properties,attempts)
		return mona.servers
	end

Indeed, the client will receive multiple server addresses, and in this case, RTMFP starts multiple connection attempt in parallel, and keep only the faster to answer. It's an other way of load-balacing system: the more faster wins.

About the P2P rendezvous service of Mona, in a multiple servers way, if the peerA connected to MonaServerA requests a connection to the peerB connected to MonaServerB, of course MonaServerA will be unable to return information about peerB. We have to use the *onRendezVousUnknown* event (see `Server Application, API <./api.html>`_ page for complete details on this event):

.. code-block:: lua

	function onRendezVousUnknown(peerId)
		return mona.servers -- redirect to all the connected servers
	end

With the above code addition, you can redirect a rendezvous request which fails to other servers.

But it's always missing a solution to synchronize member of groups in NetGroup_ usage case. Indeed, a groupA can exists on serverA and contains peerA, and the same groupA can exists on serverB too and contains peerB. peerB and peerA will never meet them. To solve it, you have to use *groups:join* method (see *groups* object description on `Server Application, API <./api.html>`_ page for complete description of this method).
The idea is simple: you have to share every group inclusion informations between all servers. The following server application code realizes this sharing job:

.. code-block:: lua

	function onRendezVousUnknown(peerId)
		return mona.servers -- redirect to all the connected servers
	end
	
	function onJoinGroup(client,group)
		-- inform other servers of this joining operation
		mona.servers:broadcast("join",group.rawId,client.rawId)
	end
	
	function onUnjoinGroup(client,group)
		-- inform other servers of this unjoining operation
		mona.servers:broadcast("unjoin",group.rawId,client.rawId)
	end
	
	function onServerConnection(server)
		-- inform this new incoming server of my group/client relations existing
		for id,group in mona.groups:pairs() do
			for i,client in mona.groups:ipairs() do
				server:send("join",group.rawId,client.rawId)
			end
		end
		
		server.groups = {}
		-- RPC server functions to receive joining/unjoining operation
		function server:join(groupId,clientId)
			-- creation of a virtual member for this group
			local member = mona:join(groupId,clientId)
			if not member then return end -- join operation has failed
			-- We have to attach this member object to its server
			-- to avoid its destruction by the LUA garbage collector
			local group = self.groups[groupId]
			if not group then self.groups[groupId] = {size=0} end
			group.size = group.size + 1
			group[clientId] = member
		end
		function server:unjoin(groupId,clientId)
			-- suppression of a possible virtual member of group
			if not group then return end
			local member = group[clientId]
			if member then
				member:release() -- detach of its group
				group[clientId] = nil
				group.size = group.size - 1
			end
			-- erase the group object if it's empty now
			if group.size==0 then self.groups[groupId]=nil end
		end
	end

	function onServerDisconnection(server)
		-- suppression of possible virtual members attached to this server
		for id,group in pairs(server.groups) do
			for id,member in pairs(group) do
				if id ~= "size" then member:release() end
			end
		end
	end

.. _NetGroup : http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/net/NetGroup.html