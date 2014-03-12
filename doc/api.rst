
.. image:: githubBlack.png
  :align: right
  :target: https://github.com/MonaSolutions/MonaServer

Mona Server's lua API
##############################

This page references objects and events available in `Server Application`_ of MonaServer.
To know how create a server application, and understand how it works, read `Server Application`_ page.

.. contents:: Table of Contents
  :depth: 3

Objects
*********

Common notions
=================

Some notions are common for all available objects.

Properties
-----------------

A property is available as follows:

.. code-block:: lua

	value = object.property


And it can be assigned if it's not a *read-only* property like that:

.. code-block:: lua
	
  object.property = value


methods
-----------------

A method is called as follows:

.. code-block:: lua

	results = object:method(arguments)


It can take multiple parameters and return mutiple results.

events
-----------------
A few objects have also some events called by the system. Simply overload the named event by defining a function as follows:

.. code-block:: lua

	function object:onManage()
		self:writeInvocation("push","test")
	end


pairs and ipairs
-----------------

Objects are kind of array or map, so you can iterate between elements of objects as follows:

.. code-block:: lua

	for index,value in ipairs(object) do
		INFO(index,value)
	end


Mona
====================

*mona* is a global object providing access to main Mona elements, available anywhere in all script files.

properties
-----------------

- **clients** (read-only), clients actually connected, see *clients* object thereafter.
- **configs**, return a LUA_ table which contains Mona configurations, it means the *MonaServer.ini* content file, (see *Configurations* part of `Installation <./installation.html>`_ page) and also some others usefull parameters (application.path, application.baseName, and many others. To know really all its content, iterate on this table and print its content). One sample is given in *Global configurations* in `Server Application`_ page.
- **environments**, return a LUA_ table which contains environment variables from the system.
- **epochTime** (read-only), gives the epoch time (since the Unix epoch, midnight, January 1, 1970) in milliseconds.
- **groups** (read-only), existing groups (NetGroup_s running), see *groups* object thereafter.
- **pulications** (read-only), server publications available, see *publications* object thereafter.
- **servers** (read-only), MonaServer instances actually connected to the server, see *Servers_* object thereafter.

example of access to a Mona global property :

.. code-block:: lua

  for id, client in pairs(mona.clients) do
		INFO(id, " : ", client.address)
	end

methods
-----------------

- **absolutePath(path)**, take in first parameter the application *path* and returns a absolute way for its folder. Helpful to separate the code of your server application in many LUA_ files (see *LUA extensions and files inclusion* part of `Server Application`_ page).
- **addToBlacklist(...)**, add to the blacklist the address(es) ip given as input argument(s).
- **removeFromBlacklist(...)**, remove from the blacklist the address(es) ip given as input argument(s).
- **createTCPClient()**, return a TCP client, see `Server Application Sockets <./serversocket.html>`_ page for more details.
- **createTCPServer()**, return a TCP server, see `Server Application Sockets <./serversocket.html>`_ page for more details.
- **createUDPSocket([allowBroadcast])**, return a UDP socket. The optional boolean *allowBroadcast* argument allows broadcasting date by this socket (by default it's to *false*). See `Server Application Sockets <./serversocket.html>`_ page for more details.
- **publish(name)**, publishs a server publication with the name given, this method returns a *Publication* object if successful, or *nil* otherwise. Indeed it can fail if a publication with the same name exists already. Read *publication* object thereafter to get more details on how push audio,video or data packet for this publication.
- **fromAMF(data)**, convert the AMF data given in parameter in multiple LUA_ types relating (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works). It returns multiple LUA_ data resulting.
- **toAMF(...)**, convert the multiple LUA_ parameters given in a AMF format (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works). It returns a string which contain data converted.
- **toAMF0(...)**, exactly same that the precedent method, but with a conversion priority to AMF0 format (when possible).
- **fromJSON(data)**, convert the JSON data given in parameter in multiple LUA_ types relating (see *JSON and LUA types conversion* part of `Server Application`_ page to know how JSON/LUA_ conversion works). It returns multiple LUA_ data resulting.
- **toJSON(...)**, convert the multiple LUA_ parameters given in a JSON format (see *JSON and LUA types conversion* part of `Server Application`_ page to know how JSON/LUA_ conversion works). It returns a string which contain data converted.
- **fromXML(data)**, convert the XML data given in parameter in multiple LUA_ types relating (see *XML and LUA types conversion* part of `Server Application`_ page to know how XML/LUA_ conversion works). It returns multiple LUA_ data resulting.
- **toXML(...)**, convert the multiple LUA_ parameters given in a XML format (see *XML and LUA types conversion* part of `Server Application`_ page to know how XML/LUA_ conversion works). It returns a string which contain data converted.
- **md5(...)**, computes and returns the MD5 values from input values given as arguments.
- **sha256(...)**, computes and returns the SHA256 values from input values given as arguments.
- **sendMail(sender,subject,content,...)**, send an email from *sender* to recipients given in the last mutiple arguments field. It returns a mail object which contains only one event, *onSent(error)* to get one notification on sent, see `Server Application Sockets <./serversocket.html>`_ page for more details.
- **split(expression,separator[,option])**, LUA_ has not real split operator, this function fills this gap. It splits the *expression* in relation with the *separator* term given, and returns tokens as a multiple result. A optional number argument indicates if you want to ignore empty tokens (*option* =1), or to remove leading and trailing whitespace from tokens (*option* =2), or the both in same time (*option* =3).
- **files(dirName)**, return a LUA_ table containing objects of type *File*, see *File_* object thereafter.

Clients
==================

*clients* object (available by *mona.clients* way, see above) is the collection of clients currently connected to the server.

methods
-----------------

- **(id/rawId)**, return a *client* object, it can take the id client parameter in a *string* format or a *raw hex* format (see *client* object thereafter).

.. note::
  
  - You can use the **ipairs()** LUA_ function to iterate on the list of *clients*, keys are *client.id* and values are *client* object (see *client* object thereafter).
  - And the "#" operator to get the number of clients.

Client
================

*client* object describes a connected client.

properties
-----------------

- **id** (read-only), the client id in a readable string format, it has a size of 64 bytes.
- **rawId** (read-only), the client id in a hexadecimal raw format, it has a size of 32 bytes.
- **address** (read-only), address of the client.
- **flashVersion** (read-only), string representing the client flash version.
- **pageUrl** (read-only), URL of the page which has gotten the connection, it means the URL of the page which contains the *SWF* client.
- **swfUrl** (read-only), URL of the *SWF* file which has gotten the connection.
- **path** (read-only), *path* used in the RTMFP URL connection, it gives server application related (see `Server Application`_).
- **ping** (read-only), client ping value.
- **writer** (read-only), the main flowWriter to communicate with the client (see *FlowWriter* object thereafter).

.. note:: *client* object can have other dynamic properties which relates HTTP properties used in the URL of connection.

.. code-block:: as3

	_netConnection.connect("rtmfp://localhost/myApplication?arg1=value1&arg2=value2");

.. code-block:: lua

	function onConnection(client,...)
		NOTE("client arg1 = "..client.arg1)
		NOTE("client arg2 = "..client.arg2)
	end


methods
-----------------

*client* has no hard-coded method by default, and if you add some methods on, you create RPC function available from client side (see *Communication between server and client* part of `Server Application`_ page for more details).

events
-----------------

- **onManage**, overloading this method allows to get an inside handle every two seconds on the related client.


FlowWriter
==================

A FlowWriter is an unidirectional communication pipe, which allows to write message in a fifo to the client. Each flowWriter is independant and have its own statistic exchange informations. It's used to communicate with the client, see *Communication between server and client* of `Server Application`_ page to get more details. Each client have at less one flowWriter opened (available by *client.writer*, see *client* object above), it's its main communication channel, but you can open many flowWriters if need.

properties
-----------------

- **reliable**, boolean to make communication server to client reliable or not. In a no-reliable case, the packet can be lost but are transfered more faster than in a reliable case. By default *reliable=true*.

.. code-block:: lua

	function onConnection(client,...)
		client.writer.reliable = false
		client.writer.writeInvocation("method","hello")  -- packet more fast but can be lost
		client.writer.reliable = true
	end

.. note:: About client to server communication this property is set on client side.

.. code-block:: as3

	_netStream.dataReliable = false
	_netStream.send("method","hello") -- packet more fast but can be lost


About stream publication it's done like that:

.. code-block:: as3

	_netStream.audioReliable = false
	_netStream.videoReliable = false
	_netStream.publish("mystream")


And  about stream subscription you opt for a no-reliable mode like that:

.. code-block:: as3

	_netStream.play("mystream",-3)

Here the server will stream in a no-reliable way and without buffering, it can improve significantly performances and better cope with congestion.

methods
-----------------

- **writeRaw(...)**, write a result for an invokation client call, it takes multiple LUA_ variables as argument to convert it to AMF and send it to the client (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works).
- **writeMessage(...)**, write a result for an invokation client call, it takes multiple LUA_ variables as argument to convert it to the output format and send it to the client (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works).
- **writeInvocation(name,...)**, invoke a client method on client side. First parameter is the name of the function to call, and then it takes multiple LUA_ variables as argument to convert it to AMF and send it to the client (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works).
- **writeStatusResponse(code,[description])**, call a status event on flash side. If code argument is *Call.Failed* for example the status event created will be *NetConnection.Call.Failed*. The second optional argument is the literal description of this event.
- **flush([full])**, fill queueing data to sending buffer to the pipe without waiting anymore. Helpful in some special real-time sending case with an important flow rate, to control sending. By default the sending is complete and immediate (*full=true*), however if the optional *full* argument equals *false* it sends only the UDP packets where no more data can be written (maximum size reached), but keep the last writing suspended. It can be usefull when you use several flowWriters, and that you want flush their data with a certain order: you call *flush(false)* on all the flowWriters, and to finish a *flush()* final to send last suspended data.
- **newFlowWriter()**, create a new flowWriter communication pipe and returns it.
- **close()**, close the communication pipe. In the case where you close a flowWriter creating by yourself (in calling *client.writer:newFlowWriter()*), it closes the flowWriter and you must not use anymore the flowWriter object which is going to be deleted. In the case where you close the main flowWriter of its client (*client.writer:close()*) it closes the entiere client session.

events
-----------------

- **onManage**, overloading this method allows to get an inside handle every two seconds (see *Communication between server and client* part of `Server Application`_ page to get a sample usage).

.. warning:: *onManage* event doesn't work for the main flowWriter of one client, but just for a flowWriter created by script code with *newFlowWriter()* method (see methods description above).


Groups
===============

Existing groups (NetGroup_s running), see *group* object thereafter.
*groups* object (available by *mona.groups* way, see above) is the collection of groups currently running on the server. It means all NetGroup_ created or joined by clients.

methods
-----------------

- **(id/rawId)**, return a *group* object, it can take the id group parameter in a *string* format or a *raw hex* format (see *group* object thereafter).
- **join(peerId,groupId)**, add the *peerId* in the group *groupId*. This feature acts on the NetGroup_ members exchange (rendezvous service), it doesn't add the client with for id *peerId* in the group, it adds the *peerId* value as a *virtual member* of the group. For this reason, you have to be sure that this peer exists somewhere and has joined this group. Indeed, it's used just in multiple-servers case (see `Scalability and load-balancing <./scalability.html>`_ page). On success it returns a *member* object related (see *member* object description below to use it).

.. note:: 
  
  - You can use the **ipairs()** LUA_ function to iterate on the list of *groups*, keys are *group.id* and values are *group* object (see *group* object thereafter).
  - And the "#" operator to get the number of groups.

Group
===============

*group* object describes a group instance (NetGroup_ instance).

properties
-----------------

- **id** (read-only), the group id in a readable string format, it has a size of 64 bytes.
- **rawId** (read-only), the group id in a hexadecimal raw format, it has a size of 32 bytes.

.. note:: 
  
  - You can use the **ipairs()** LUA_ function to iterate on the list of *clients*.
  - And the "#" operator to get the number of clients.

Member
=================

*member* object is a virtual member of one group, gotten by a *groups:join(peerId,joinId)* call (see *groups* object above). It's here just to allow to detach this virtual member of its group. It's done on its destruction by the LUA_ garbage collector, or when wanted in calling its *release()* method.

properties
-----------------

- **id** (read-only), the peer id in a readable string format, it has a size of 64 bytes.
- **rawId** (read-only), the peer id in a hexadecimal raw format, it has a size of 32 bytes.

methods
-----------------

- **release()**, unjoin its group, its existence has no more meaning.


QualityOfService
=========================

*qualityOfService* object describes describes how are going a publication or a subscription (see *publication* and *listener* objects thereafter).

properties
-----------------

- **byteRate** (read-only), byte rate (bytes per second).
- **lostRate** (read-only), value between 0 and 1 to indicate the lost data rate.
- **congestionRate** (read-only), value between -1 and 1 to indicate the congestion data rate. When value is negative it means that byte rate could certainly be increased because there is available bandwith (*-0.5* means that a byte rate increased of 50% is certainly possible).
- **latency** (read-only), delay in milliseconds between data sending and receiving .
- **droppedFrames** (read-only), only available in a video stream, indicate number of frames removed by MonaServer to wait new key frame on lost data (on stream configured in a not reliable mode), or on new subscription when the publication is live-streaming.

Publications
====================

*publications* object (available by *mona.publications* way, see above) is the collection of publications actually publishing on the server.

methods
-----------------

- **(name)**, return a *publication* object, it can take the name of the publication in argument (see *publication* object thereafter).

.. note:: 
  
  - You can use the **ipairs()** LUA_ function to iterate on the list of *publications*, keys are *publication.name* and values are *publication* object (see *publication* object thereafter).
  - And the "#" operator to get the number of publications.


Publication
=================

*publication* object describes a publication.

properties
-----------------

- **name** (read-only), name of the publication
- **publisherId** (read-only), unique identifier the publisher.
- **listeners** (read-only), listeners which have subscribed for this publication, see *listeners* object thereafter.
- **audioQOS** (read-only), *qualityOfService* object about audio transfer for this publication (see *qualityOfService* object above).
- **videoQOS** (read-only), *qualityOfService* object about video transfer for this publication (see *qualityOfService* object above).

methods
-----------------

- **pushAudioPacket(time,packet[,offset,lost])**, push audio data to this publication. First argument is the time in milliseconds of this audio sample in the stream, the second argument is the packet data. The third optional argument allows to give an offset beginning position on the packet given (0 by default), and the last optional argument is to indicate the number of lost packets gotten since the last call for this method (it's used by *qualityOfService* object, see above).
- **pushVideoPacket(time,packet[,offset,lost])**, push video data to this publication. First argument is the time in milliseconds of this video frame in the stream, the second argument is the packet data. The third optional argument allows to give an offset beginning position on the packet given (0 by default), and the last optional argument is to indicate the number of lost packets gotten since the last call for this method (it's used by *qualityOfService* object, see above).
- **pushDataPacket(name,packet[,offset])**, push named data to this publication. First argument is the name of this data which relates methods to invoke on listeners side, second argument is the packet data, and the third optional argument allows to give an offset beginning position on the packet given (0 by default),
- **flush()**, when you push audio, video or data packets, they are not flushed to listeners, you have to call this method to broadcast data when you have finished of pushed all available packets.
- **close([code,description])**, close a publication. If this publication had been published with *mona.publish* function (see *Mona* object above), the publication will be unpublished and deleted, and optional arguments are useless. Now if it's a client publication, its method *close* will be invoked, and a status event will be sent if optional arguments are indicated. For example, *publication:close('Publish.Error','error message')* will send a *NetStream.Publish.Error* statut event with as description *error message*, and then will invoke *close* method on client side for the *NetStream* object related.


Listeners
=================

*listeners* object describes a collection of subscribers for one publication (see *publication* object above).

.. note:: 
  
  - You can use the **ipairs()** LUA_ function to iterate on the list of *listener* (see *listener* object thereafter).
  - And the "#" operator to get the number of listeners.


Listener
=================

*listener* object describes a subscriber for one publication.

properties
-----------------

- **id** (read-only), unique identifier for the listener.
- **audioQOS** (read-only), *qualityOfService* object about audio transfer for this subscription (see *qualityOfService* object above).
- **videoQOS** (read-only), *qualityOfService* object about video transfer for this subscription (see *qualityOfService* object above).
- **publication** (read-only), *publication* object which describes publication listening by the subscriber (see *publication* object above).
- **audioSampleAccess**, boolean to authorize or not audio sample access by the subscriber (see `NetStream:audioSampleAccess <http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/net/NetStream.html#audioSampleAccess>`_ property).
- **videoSampleAccess**, boolean to authorize or not video sample access by the subscriber (see `NetStream:audioSampleAccess <http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/net/NetStream.html#audioSampleAccess>`_ property).
- **receiveAudio**, boolean to mute audio reception on the subscription.
- **receiveVideo**, boolean to mute video reception on the subscription.


ByteReader
=================

This object is only used for IExternalizable typed object, it's the first argument of *__readExternal* function, and it's an equivalent for IDataInput_ AS3 class (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works). It contains exactly same functions, excepting *readObject()* which is replaced by *readAMF(x)* function. *readAMF(x)* returns the *x* first LUA_ results which come from the AMF unserialization.


ByteWriter
=================

This object is only used for IExternalizable typed object, it's the first argument of *__writeExternal* function, and it's an equivalent for IDataOutput_ AS3 class (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works). It contains exactly same functions, excepting _writeObject(object:*)_ which is replaced by *writeAMF(...)* function. *writeAMF(...)* takes multiple LUA_ arguments for AMF serialization.


Servers
=================

Servers list of MonaServer currently connected to the server (see `Scalability and load-balancing <./scalability.html>`_ page for more details about multiple server features).

properties
-----------------

- **initiators** (read-only), return a *broadcaster* object (see thereafter) including the server initiators. Server connections have a direction, with an iniator of the connection, and a target of the connection (see `Scalability and load-balancing <./scalability.html>`_ page for more details). 
- **targets** (read-only), return a *broadcaster* object (see thereafter) including the server targets. Server connections have a direction, with an iniator of the connection, and a target of the connection (see `Scalability and load-balancing <./scalability.html>`_ page for more details). 

methods
-----------------

- **broadcast(handler,...)**, broadcast data to servers (initiators and targets). The *handler* parameter is the name of the *remote procedure call* method to receive data, multiple arguments following are the data (see `Scalability and load-balancing <./scalability.html>`_ page for more details).
- **(address/index)**, return a *server* object (see *server* object thereafter). It can take the *address* of the server (string format) or the *index* of server (number format) as input argument. Indeed the list is sorted by order of connections.

.. note:: 
  
  - You can use the **ipairs()** LUA_ function to iterate on the list of servers.
  - And the "#" operator to get the number of servers.

Server
===========

*server* object describes a server communication (see `Scalability and load-balancing <./scalability.html>`_ page for more details about multiple server features).

properties
-----------------

- **address** (read-only), name of the publication
- **publicAddress** (read-only), name of the publication
- **isTarget** (read-only), name of the publication

.. note:: *server* object can have other dynamic properties (as *client* object) which relates properties used during the server connection (see *Configurations* part of `Installation <./installation.html>`_ page).

methods
-----------------

*Server* has no hard-coded method by default, and if you add some methods on, you create RPC function available from other server (see `Scalability and load-balancing <./scalability.html>`_ page for more details).


Broadcaster
==============

Allow to manipulate list of server initiators or targets gotten with *servers.initiators* or *servers.targets* (see *servers* object below).

properties
-----------------

- **count** (read-only), number of servers.

methods
-----------------

- **broadcast(handler,...)**, broadcast data to servers. The *handler* parameter is the name of the *remote procedure call* method to receive data, multiple arguments following are the data (see `Scalability and load-balancing <./scalability.html>`_ page for more details).
- **(address/index)**, return a *server* object (see *server* object thereafter). It can take the *address* of the server (string format) or the *index* of server (number format) as input argument. Indeed the list is sorted by order of connections.

.. note:: You can use the **ipairs()** LUA_ function to iterate on the list of servers.

File
=============

*File* object gives some properties of a file in the file system. *File* objects are created on a *mona:files(...)* call.

properties
-----------------

- **fullPath** (read-only), full path of the file
- **name** (read-only), name of the file
- **baseName** (read-only), name of the file, without extension
- **extension** (read-only), extension of the file
- **size** (read-only), size of the file
- **lastModified** (read-only), date of last modification (in seconds)
- **isDirectory** (read-only), true if the file is a directory

Events
**************

MonaServer calls some events in application server script.


Common notions
===================

All event names starts with the *on* prefix.

.. code-block:: lua

	function onConnection(client,...)
	end


onStart(path)
===================

Call when the server application is built and executed the first time. The first argument is the *path* of the application (see *Create a server application* part of `Server Application`_ page).

.. warning:: All server application are built on first client connection for the application, except *root* application (*/* application), which is started on MonaServer starting.


onStop(path)
=====================

Call when the server application is unloaded. It happens in three different cases:

- When you edit *main.lua* file of one server application. Application is restarted (stopped and started).
- When you delete a server application.
- When MonaServer is stopping.

The first argument is the *path* of the application (see *Create a server application* part of `Server Application`_ page).


onConnection(client,...)
=============================

Call on a new client connection. First argument is a client object (see *client* object description above), and following multiple arguments are AMF parameters given to *NetConnection:connect(address:String, ... parameters)* converted in LUA_ types (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works).

Finally you can return a table result to add some informations on connection (always with *AMF and LUA types conversion*, see `Server Application`_ page):

.. code-block:: lua

	function onConnection(client,...)
		return {message="welcome",id=1}
	end

.. code-block:: as3

	function onStatusEvent(event:NetStatusEvent):void {
		switch(event.info.code) {
			case "NetConnection.Connect.Success":
			trace(event.info.message); // displays "welcome"
			trace(event.info.id); // displays "1"
			break;
		}
	}

You can reject a client adding an error of connection:

.. code-block:: lua

	function onConnection(client,login)
		if login ~= "Tom" then
			error("you are not Tom!")
		end
	end

.. code-block:: as3

	_netConnection.connect("rtmfp://localhost/","Ben")

	function onStatusEvent(event:NetStatusEvent):void {
		switch(event.info.code) {
			case "NetConnection.Connect.Rejected":
			trace(event.info.description); // displays "you are not Tom!"
			break;
		}
	}

It answers with a *NetConnection.Connect.Rejected* status event and close the client connection. The *event.info.description* field contains your error message. Now if you reject a client with no error message, *event.info.description* field will contain "client rejected" by default.

.. code-block:: lua

	function onConnection(client,...)
		error("")
	end

.. code-block:: as3

	_netConnection.connect("rtmfp://localhost/")

	function onStatusEvent(event:NetStatusEvent):void {
		switch(event.info.code) {
			case "NetConnection.Connect.Rejected":
			trace(event.info.description); // displays "client rejected"
			break;
		}
	}

onDisconnection(client)
============================

Call on client disconnection. *client* argument is the disconnected client.

.. note:: At this stage you can send no more data to the client, all writing to a flowWriter object of this client has no effect.


onFailed(client,error)
============================

Call on client failing. *client* argument is the failed client, and *error* argument is the error message.

.. note:: At this stage you can send no more data to the client, all writing for a flowWriter object of this client has no effect.


onJoinGroup(client,group)
==============================

Call when a client creates or joins a *group* (a NetGroup_). *client* argument is the client which is joining the *group* second argument (see *group* object in *Objects* part).

onUnjoinGroup(client,group)
===============================

Call when a client unjoins a *group* (a NetGroup_). *client* argument is the client which is unjoining the *group* second argument (see *group* object in *Objects* part).

onPublish(client,publication)
===============================

Call when a publication starts. *client* is the client which starts the publication, and *publication* argument is the publication description (see *publication* object in *Objects* part).

If you return *false* value on this event, it will send a *NetStream.Publish.Failed* status event with as *info.description* field a *"Not allowed to publish [name]"* message.
Otherwise you can cutomize this message in raising one error in this context.

.. code-block:: lua

	function onPublish(client,publication)
		if not client.right then
			error("no rights to publish it")
		end
	end

.. code-block:: as3
	
  function onStatusEvent(event:NetStatusEvent):void {
		switch(event.info.code) {
			case "NetStream.Publish.Failed":
			trace(event.info.description); // displays "no rights to publish it"
			break;
		}
	}

.. warning:: This event is not called for publications started from script code, it's called only for client publications (see *publication* object in *Objects* part). Then of course, it's called only in stream-to-server case (not in P2P case).


onUnpublish(client,publication)
=====================================

Call when a publication stops. *client* is the client which have stopped the publication, and *publication* argument is the publication related.

.. warning:: This event is not called for publications started from script code, it's called only for client publications (see *publication* object in *Objects* part). Then of course, it's called only in stream-to-server case (not in P2P case).


onVideoPacket(client,publication,time,packet)
==================================================

Call on video packet reception for one publication. *time* is the time in milliseconds of this packet in the stream, and *packet* contains video data.

.. warning:: This event is not called for publications started from script code, it's called only for client publications (see *publication* object in *Objects* part). Then of course, it's called only in stream-to-server case (not in P2P case).


onAudioPacket(client,publication,time,packet)
===================================================

Call on audio packet reception for one publication. *time* is the time in milliseconds of this packet in the stream, and *packet* contains audio data.

.. warning:: This event is not called for publications started from script code, it's called only for client publications (see *publication* object in *Objects* part). Then of course, it's called only in stream-to-server case (not in P2P case).


onDataPacket(client,publication,name,packet)
===================================================

Call on data packet reception for one publication. *name* is the invocation name, and *packet* contains raw data.

.. warning:: This event is not called for publications started from script code, it's called only for client publications (see *publication* object in *Objects* part). Then of course, it's called only in stream-to-server case (not in P2P case).

onSubscribe(client,listener)
==============================

Call on new client subscription. First *client* argument is the client which starts the stream subscription, and *listener* describes the subscription (see *listener* object in *Objects* part).

If you return *false* value on this event, it will send a *NetStream.Play.Failed* status event with as *info.description* field a *"Not authorized to play [name]"* message.
Otherwise you can cutomize this message in raising one error in this context.

.. code-block:: lua

	function onSubscribe(client,listener)
		if not client.right then
			error("no rights to play it")
		end
	end

.. code-block:: as3

	function onStatusEvent(event:NetStatusEvent):void {
		switch(event.info.code) {
			case "NetStream.Play.Failed":
			trace(event.info.description); // displays "no rights to play it"
			break;
		}
	}

.. warning::

 - This event is called only in stream-to-server case (not in P2P case).
 - The listener is added to the *listener.publication.listeners* list after this call, so the value *listener.publication.listeners.count* will return the old value, and only if onSubscribe accepts the new listener, will be incremented.

onUnsubscribe(client,listener)
=====================================

Call on client unsubscription. First *client* argument is the client which stops the stream subscription, and *listener* describes the subscription closed (see *listener* object in *Objects* part).

.. warning::

 - This event is called only in stream-to-server case (not in P2P case).
 - The listener is removed to the *listener.publication.listeners* list after this call, so the value *listener.publication.listeners.count* will return the old value until the end of this call.

onTypedObject(type,object)
=====================================

This event is called on AMF unserialization of a typed object, it allows to link your own LUA_ type with AMF typed object, or also customizes AMF serialization/unserialization. See *AMF and LUA types conversion* part of `Server Application`_ page for more details.

onManage()
=====================================

Call every two seconds, this event is available only in the *root* server application (*www/main.lua*). It allows easyly to get handle to manage your objects if need.


onRendezVousUnknown(peerId)
=====================================

Allows to redirect a client who searchs a peerId that the rendezvous service doesn't find. Usually you will redirect the client to one or multiple other MonaServer (see `Scalability and load-balancing <./scalability.html>`_ for more details on multiple servers usage). You can return an address, but also multiple address, or an array of addresses.

.. code-block:: lua

	function onRendezVousUnknown(peerId)
		return 192.168.0.2:1935
	end

.. code-block:: lua

	function onRendezVousUnknown(peerId)
		return 192.168.0.2:1935,192.168.0.3:1935
	end

.. code-block:: lua

	addresses = {192.168.0.2:1936,192.168.0.3:1936}
	function onRendezVousUnknown(peerId)
		return addresses
	end

Then you can return a *server* object or a *servers* object (see above for these object descriptions):

.. code-block:: lua
	
  function onRendezVousUnknown(peerId)
		return mona.servers[1] -- redirect to the first server connected
	end

.. code-block:: lua

	function onRendezVousUnknown(peerId)
		return mona.servers -- redirect to all the connected servers
	end

.. note:: When this function returns multiple addresses, the client will receive all these addresses and will start multiple attempt in parallel to these servers.


onHandshake (address,path,properties,attempts)
===============================================

Allows to redirect the client to one other MonaServer (see `Scalability and load-balancing <./scalability.html>`_ for more details on multiple servers usage), in returning address(es) of redirection. About the returned value it works exactly same the returned value of *onRendezVousUnknown* event (see above).
It's called on the first packet received from one client (before the creation of its client object associated). First *address* argument is the address of the client, *path* argument indicates the path expression of connection, *properties* argument is a table with the HTTP parameters given in the URL of connection (see dynamic properties of *client* object description above) and *attempts* argument indicates the number of attempts of connection (starts to 1 and is incremented on each attempt).

.. code-block:: as3

	_netConnection.connect("rtmfp://localhost/myApplication?acceptableAttempts=2");

.. code-block:: lua

	index=0
	function onHandshake(address,path,properties,attempts)
		if attempts > properties.acceptableAttempts then
			-- This time we return all server available,
			-- and it's the client who will test what is the server the faster with parallel connection
			-- (first which answers wins)
			return mona.servers
		end
		index=index+1
		if index > mona.servers.count then index=1 end -- not exceed the number of server available
		return mona.servers[index] -- load-balacing system!
	end

.. note:: You can use the keyword *again* to request a new attempt on *myself* (if the other redirection doesn't work).

.. code-block:: lua

	function onHandshake(address,path,properties,attempts)
		return mona.servers,"again" -- redirect to the other server and my myself
	end

onServerConnection(server)
====================================

Call on server connection, see `Scalability and load-balancing <./scalability.html>`_ for more details on multiple servers usage, or *server* object description above about the input argument.


onServerDisconnection(server)
====================================

Call on server disconnection, see `Scalability and load-balancing <./scalability.html>`_ for more details on multiple servers usage, or *server* object description above about the input argument.


.. _LUA: http://www.lua.org/
.. _NetGroup: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/net/NetGroup.html
.. _IDataOutput: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IDataOutput.html
.. _IDataInput: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IDataInput.html
.. _Server Application: ./serverapp.html