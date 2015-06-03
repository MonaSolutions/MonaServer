
API
##############################

This page references objects and events availables in a :doc:`serverapp` of MonaServer.

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

Application properties
========================

- **name** (read-only), the name of the application.
- **path** (read-only), the path of the current application.
- **this** (read-only), the current application's object.
- **super** (read-only), an object giving acces to members and functions of the parent application.
- **children** (read-only), an associative array containing children applications as values and their names as keys (only already loaded children are availables).

See :doc:`serverapp` to have more information of communication between applications.

.. _ref-mona:

Mona
====================

*mona* is a global object providing access to main Mona elements, available anywhere in all script files.

properties
-----------------

- **clients** (read-only), Clients_ actually connected.
- **configs** (read-only), return a LUA_ table which contains Mona configurations, it means the *MonaServer.ini* content file, (see :ref:`ref-configurations`) and also some others usefull parameters (application.path, application.baseName, and many others. To know really all its content, iterate on this table and print its content). One sample is given in :ref:`ref-global-configurations`.
- **environment** (read-only), return a LUA_ table which contains environment variables from the system.
- **groups** (read-only), existing Groups_ (NetGroup_ running).
- **pulications** (read-only), server Publications_ available.
- **servers** (read-only), MonaServer instances actually connected to the server, see Servers_ object.

Example of access to a Mona global property :

.. code-block:: lua

  for id, client in pairs(mona.clients) do
    INFO(id, " : ", client.address)
  end

methods
-----------------

- **absolutePath(path)**, take in first parameter the application *path* and returns a absolute way for its folder. Helpful to separate the code of your server application in many LUA_ files (see :ref:`ref-lua-extensions`).
- **addToBlacklist(...)**, add to the blacklist the address(es) ip given as input argument(s).
- **removeFromBlacklist(...)**, remove from the blacklist the address(es) ip given as input argument(s).
- **createIPAddress(address)**, convert an IP Address string to an IPAddress_ object without DNS resolution (not blocking method).
- **createIPAddressWithDNS(address)**, convert an IP Address string to an IPAddress_ object with DNS resolution (blocking method).
- **createSocketAddress(address)**, convert a Socket Address string (with host and port) to a SocketAddress_ object without DNS resolution (not blocking method).
- **createSocketAddressWithDNS(address)**, convert a Socket Address string (with host and port) to a SocketAddress_ object with DNS resolution (blocking method).
- **createTCPClient()**, return a TCP client, see :doc:`serversocket` page for more details.
- **createTCPServer()**, return a TCP server, see :doc:`serversocket` page for more details.
- **createUDPSocket([allowBroadcast])**, return a UDP socket. The optional boolean *allowBroadcast* argument allows broadcasting date by this socket (by default it's to *false*). See :doc:`serversocket` page for more details.
- **publish(name)**, publishs a server publication with the name given, this method returns a Publication_ object if successful, or *nil* otherwise. Indeed it can fail if a publication with the same name exists already. Read Publication_ object thereafter to get more details on how push audio,video or data packet for this publication.
- **fromAMF(data)**, convert the AMF data given in parameter in multiple LUA_ types relating (see :ref:`ref-amf-to-lua` to know how AMF/LUA_ conversion works). It returns multiple LUA_ data variables.
- **toAMF(...)**, convert the multiple LUA_ parameters given in a AMF format (see :ref:`ref-amf-to-lua` to know how AMF/LUA_ conversion works). It returns a string which contain data converted.
- **toAMF0(...)**, exactly same that the precedent method, but with a conversion priority to AMF0 format (when possible).
- **fromJSON(data)**, convert the JSON data given in parameter in multiple LUA_ types relating (see :ref:`ref-json-to-lua` to know how JSON/LUA_ conversion works). It returns multiple LUA_ data variables.
- **toJSON(...)**, convert the multiple LUA_ parameters given in a JSON format (see :ref:`ref-json-to-lua` to know how Query/LUA_ conversion works). It returns a string which contain data converted.
- **fromQuery(data)**, convert the `Query string`_ data given in parameter in multiple LUA_ types relating (see :ref:`ref-json-to-lua` to know how Query/LUA_ conversion works). It returns multiple LUA_ data variables.
- **toQuery(...)**, convert the multiple LUA_ parameters given in a `Query string`_ format (see :ref:`ref-json-to-lua` to know how JSON/LUA_ conversion works). It returns a string which contain data converted.
- **fromXML(data)**, convert the XML data given in parameter in multiple LUA_ types relating (see :ref:`ref-xml-compatibility` to know how XML/LUA_ conversion works). It returns multiple LUA_ data variables.
- **toXML(...)**, convert the multiple LUA_ parameters given in a XML format (see :ref:`ref-xml-compatibility` to know how XML/LUA_ conversion works). It returns a string which contain data converted.
- **fromXMLRPC(data)**, convert the XML-RPC_ data given in parameter in multiple LUA_ types relating (see :ref:`ref-xmlrpc-to-lua` to know how XMLRPC/LUA_ conversion works). It returns multiple LUA_ data variables.
- **toXMLRPC(...)**, convert the multiple LUA_ parameters given in a XML-RPC_ format (see :ref:`ref-xmlrpc-to-lua` to know how XMLRPC/LUA_ conversion works). It returns a string which contain data converted.
- **md5(...)**, computes and returns the MD5 values from input values given as arguments.
- **sha256(...)**, computes and returns the SHA256 values from input values given as arguments.
- **split(expression,separator[,option])**, LUA_ has not real split operator, this function fills this gap. It splits the *expression* in relation with the *separator* term given, and returns tokens as a multiple result. A optional number argument indicates if you want to ignore empty tokens (*option* =1), or to remove leading and trailing whitespace from tokens (*option* =2), or the both in same time (*option* =3).
- **listPaths(dirName)**, return a LUA_ table containing objects of type File_ in the *dirName* directory (relative to the **www** path).
- **joinGroup(peerID, groupID)**, add Client_ with *peerID* to Group_ with *groupID*.
- **time()**, gives the epoch time (since the Unix epoch, midnight, January 1, 1970) in milliseconds.
- **dump(data[, size])**, dump data to the console and log file, if *size* is not specified it dump all the data.

Example of access to a Mona global function :

.. code-block:: lua

  # Print congiguration array in a JSON format
  INFO(mona:toJSON(mona.configs))

Data
==================

**data** is the global variable that permits you to have persistent values, see :doc:`database` page to know how to use it.

Clients
==================

*clients* object (available by *mona.clients* way, see Mona_ object) is the collection of clients currently connected to the server.

methods
-----------------

- **(id/rawId)**, return a Client_ object, it can take the id client parameter in a *string* format or a *raw hex* format.

.. note::
  
  - You can use the **pairs()** LUA_ function to iterate on the list of *clients*, keys are *client.id* and values are Client_ objects.
  - And the "#" operator to get the number of clients.

.. _ref-client:

Client
================

*client* object describes a connected client.

properties
-----------------

- **id** (read-only), the client id in a readable string format, it has a size of 64 bytes.
- **rawId** (read-only), the client id in a hexadecimal raw format, it has a size of 32 bytes.
- **address** (read-only), address of the client.
- **path** (read-only), *path* used in the URL connection, it gives server application related (see :doc:`serverapp`).
- **ping** (read-only), client ping value.
- **lastReceptionTime** (read-only), time of last data reception.
- **protocol** (read-only), client protocol name (HTTP, WebSocket, RTMP or RTMFP).
- **query** (read-only), query part of the url (used in HTTP).
- **writer** (read-only), the main Writer_ to communicate with the client.
- **properties** (read-only), dynamic properties of the client connection, depends on the protocol (see :doc:`protocols`).
- **parameters** (read-only), static parameters/configuration of the client protocol (**parameters** can be substituate by protocol name).

.. note::

  - You can use the **pairs()** LUA_ function to iterate on the lists *client.properties* and *client.parameters*.
  - And the "#" operator to get the number of properties/parameters.

In *client.properties* the word *properties* can be omitted to access directly to client's attributes. Here is a sample with an RTMFP connection :

.. code-block:: as3

  _netConnection.connect("rtmfp://localhost/myApplication?arg1=value1&arg2=value2");

.. code-block:: lua

  function onConnection(client,...)
    NOTE("client arg1 = "..client.arg1)
    NOTE("client arg2 = "..client.arg2)
  end


methods
-----------------

*client* has no hard-coded method by default, and if you add some methods on, you create RPC function available from client side (see :ref:`ref-com-server-client` for more details).


events
-----------------

- **onAddressChanged(oldAddress)**, happen on client.address change (can happen for an UDP based protocol like RTMFP).
- **onJoinGroup(group)**, happen when the client join a P2P Group_ (RTMFP - NetGroup). 
- **onUnjoinGroup(group)**, happen when the client leaves a P2P Group_ (RTMFP - NetGroup).
- **onPublish(publication)**, happen on Publication_ starts.

If you return *false* value on this event, it will send a *NetStream.Publish.Failed* status event with as *info.description* field a *"Not allowed to publish [name]"* message.
Otherwise you can cutomize this message in raising one error in this context.

.. code-block:: lua

  function client:onPublish(publication)
    if not client.right then
      error("no rights to publish it")
    end
    
    function publication:onData(time,packet)
      -- write code here
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

.. warning:: This event is not called for publications started from script code, it's called only for client publications (see Publication_ object). Then of course, it's called only in stream-to-server case (not in P2P case).

- **onUnpublish(publication)**, happen on Publication_ stops.

.. warning:: This event is not called for publications started from script code, it's called only for client publications (see Publication_ object). Then of course, it's called only in stream-to-server case (not in P2P case).

- **onSubscribe(listener)**, happen on publication subscription, Listener_ argument describes this subscription.

If you return *false* value on this event, it will send a *NetStream.Play.Failed* status event with as *info.description* field a *"Not authorized to play [name]"* message.
Otherwise you can cutomize this message in raising one error in this context.

.. code-block:: lua

  function client:onSubscribe(listener)
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

.. warning:: This event is called only in stream-to-server case (not in P2P case).
 
- **onUnsubscribe(listener)**, happen on publication unsubscription, Listener_ argument describes the abandoned subscription.

.. warning:: This event is called only in stream-to-server case (not in P2P case).
 
- **onRead(file,parameters...)**

This event is used with **HTTP** protocol.

Called when a client try to read a file on the server. The file should exists.
Parameters should be used to perform REST functionnalities.

You can also reject the connexion like this :

.. code-block:: lua

  function client:onRead(file)
    if file ~= "index.html" then
      error("Access to file ", file, " is forbidden)
    end
  end

You can redirect to another file returning the file name as first parameter :

.. code-block:: lua

  function client:onRead(file)
    return "newFile"
  end
  
Other parameters are treated as values for replacing templates *<% property %>* in file. So with the script below each *<% name %>* element will be replaced by "robert" :

.. code-block:: lua

  function client:onRead(file)
    return file, {name="robert"}
  end

If you need to return a custom response you can return *nil* and write you response using the writer as below:

.. code-block:: lua

  function client:onRead(file,parameters)
    self.writer:writeRaw("hello"); -- my custom response
    return nil
  end

.. note:: You can create your own events on client object to create RPC server methods. These methods will be accessible from client side. By default if you send data to server from client without given a method name, the method invoked is *client:onMessage*. See :ref:`ref-com-server-apps` for more details.
 

.. _ref-writer:

Writer
==================

A Writer is an unidirectional communication pipe, which allows to write message in a fifo to the client. Each writer is independant and have its own statistic exchange informations. It's used to communicate with the client, see :ref:`ref-com-server-client` to get more details. Each Client_ have at least one Writer_ opened (available by *client.writer*), it's its main communication channel, but you can open many writers if need.

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

- **writeRaw(...)**, write a result for an invokation client call, it takes multiple LUA_ variables as argument to convert it to AMF and send it to the client (see :ref:`ref-amf-to-lua` to know how AMF/LUA_ conversion works).
- **writeMessage(...)**, write a result for an invokation client call, it takes multiple LUA_ variables as argument to convert it to the output format and send it to the client (see :ref:`ref-amf-to-lua` to know how AMF/LUA_ conversion works).
- **writeInvocation(name,...)**, invoke a client method on client side. First parameter is the name of the function to call, and then it takes multiple LUA_ variables as argument to convert it to AMF and send it to the client (see :ref:`ref-amf-to-lua` to know how AMF/LUA_ conversion works).
- **writeStatusResponse(code,[description])**, call a status event on flash side. If code argument is *Call.Failed* for example the status event created will be *NetConnection.Call.Failed*. The second optional argument is the literal description of this event.
- **flush([full])**, fill queueing data to sending buffer to the pipe without waiting anymore. Helpful in some special real-time sending case with an important flow rate, to control sending. By default the sending is complete and immediate (*full=true*), however if the optional *full* argument equals *false* it sends only the UDP packets where no more data can be written (maximum size reached), but keep the last writing suspended. It can be usefull when you use several writers, and that you want flush their data with a certain order: you call *flush(false)* on all the writers, and to finish a *flush()* final to send last suspended data.
- **newWriter()**, create a new writer communication pipe and returns it.
- **close()**, close the communication pipe. In the case where you close a writer creating by yourself (in calling *client.writer:newWriter()*), it closes the writer and you must not use anymore the writer object which is going to be deleted. In the case where you close the main writer of its client (*client.writer:close()*) it closes the entiere client session.

events
-----------------

- **onManage**, overloading this method allows to get an inside handle every two seconds (see :ref:`ref-com-server-client` to get a sample usage).

.. warning:: *onManage* event doesn't work for the main writer of one client, but just for a writer created by script code with *newWriter()* method (see methods description above).

.. _ref-groups:

Groups
===============

Existing groups (NetGroup_ running), see Group_ object.
*groups* object (available by *mona.groups* way, see above) is the collection of groups currently running on the server. It means all NetGroup_ created or joined by clients.

methods
-----------------

- **(id/rawId)**, return a Group_ object, it can take the id group parameter in a *string* format or a *raw hex* format (see Group_ object).
- **join(peerId,groupId)**, add the *peerId* in the group *groupId*. This feature acts on the NetGroup_ members exchange (rendezvous service), it doesn't add the client with for id *peerId* in the group, it adds the *peerId* value as a *virtual member* of the group. For this reason, you have to be sure that this peer exists somewhere and has joined this group. Indeed, it's used just in multiple-servers case (see :doc:`scalability` page). On success it returns a Member_ object related.

.. note:: 
  
  - You can use the **pairs()** LUA_ function to iterate on the list of Groups_, keys are *group.id* and values are Group_ objects.
  - And the "#" operator to get the number of groups.

Group
===============

*group* object describes a group instance (NetGroup_ instance).

properties
-----------------

- **id** (read-only), the group id in a readable string format, it has a size of 64 bytes.
- **rawId** (read-only), the group id in a hexadecimal raw format, it has a size of 32 bytes.
- **members** (read-only), the list of Clients_ in the group.

methods
-----------------

- **size()**, return the number of client of the group.

.. note:: 
  
  - You can use the **pairs()** LUA_ function to iterate on the list of *members*.

Member
=================

*member* object is a virtual member of one group, gotten by a *groups:join(peerId,joinId)* call (see Groups_ object). It's here just to allow to detach this virtual member of its group. It's done on its destruction by the LUA_ garbage collector, or when wanted in calling its *release()* method.

properties
-----------------

- **id** (read-only), the peer id in a readable string format, it has a size of 64 bytes.
- **rawId** (read-only), the peer id in a hexadecimal raw format, it has a size of 32 bytes.

methods
-----------------

- **release()**, unjoin its group, its existence has no more meaning.


QualityOfService
=========================

*qualityOfService* object describes describes how are going a publication or a subscription (see Publication_ and Listener_ objects).

properties
-----------------

- **lostRate** (read-only), value between 0 and 1 to indicate the lost data rate.
- **byteRate** (read-only), byte rate (bytes per second).
- **latency** (read-only), delay in milliseconds between data sending and receiving.
- **lastSendingTime** (read-only), last time a frame has been sent.

Publications
====================

*publications* object (available by *mona.publications* way, see Mona_ object) is the collection of publications actually publishing on the server.

methods
-----------------

- **(name)**, return a Publication_ object, it can take the name of the Publication_ in argument.

.. note:: 
  
  - You can use the **pairs()** LUA_ function to iterate on the list of Publications_, keys are *publication.name* and values are Publication_ object.
  - And the "#" operator to get the number of publications.


Publication
=================

*publication* object describes a publication.

properties
-----------------

- **name** (read-only), name of the publication
- **lastTime** (read-only), returns the last media time published.
- **droppedFrames** (read-only), return the number of dropped video frames till the publication is alive. Frames are dropped when at least one fragment is lost.
- **listeners** (read-only), Listeners_ which have subscribed for this publication.
- **audioQOS** (read-only), QualityOfService_ object about audio transfer for this publication.
- **videoQOS** (read-only), QualityOfService_ object about video transfer for this publication.
- **dataQOS** (read-only), QualityOfService_ object about data transfer for this publication.
- **running** (read-only), true if the publication is running.
- **properties** (read-only), returns publication properties (metadata).

methods
-----------------

- **writeProperties(properties)**, set publication properties (write metadata).
- **clearProperties()**, clear publication properties (clear metadata).
- **pushAudio(time,packet[,ping])**, push audio data to this publication. First argument is the time in milliseconds of this audio sample in the stream, the second argument is the packet data. And the last optional argument is to increment the latency counter (it's used by QualityOfService_ object).
- **pushVideo(time,packet[,ping])**, push video data to this publication. First argument is the time in milliseconds of this video frame in the stream, the second argument is the packet data. And the last optional argument is to increment the latency counter (it's used by QualityOfService_ object).
- **pushAMFData(...)**, push data to this publication. The arguments passed are serialized in AMF.
- **pushAMF0Data(...)**, push data to this publication. The arguments passed are serialized in AMF0.
- **pushXMLRPCData(...)**, push data to this publication. The arguments passed are serialized in XMLRPC.
- **pushJSONData(...)**, push data to this publication. The arguments passed are serialized in JSON.
- **pushData(...)**, push data to this publication. The arguments passed are just raw concatenated (no typed serialization).
- **flush()**, when you push audio, video or data packets, they are not flushed to listeners, you have to call this method to broadcast data when you have finished of pushed all available packets.
- **close([code,description])**, close a publication. If this publication had been published with *mona.publish* function (see Mona_ object), the publication will be unpublished and deleted, and optional arguments are useless. Now if it's a client publication, its method *close* will be invoked, and a status event will be sent if optional arguments are indicated. For example, *publication:close('Publish.Error','error message')* will send a *NetStream.Publish.Error* statut event with as description *error message*, and then will invoke *close* method on client side for the *NetStream* object related.

events
-----------------

- **onVideo(time,packet)**, call on video packet reception for one Publication_. *time* is the time in milliseconds of this packet in the stream, and *packet* contains video data.
- **onAudio(time,packet)**, call on audio packet reception for one Publication_. *time* is the time in milliseconds of this packet in the stream, and *packet* contains video data.
- **onData(packet)**, call on data packet reception for one Publication_. *packet* contains raw data.
- **onFlush()**, call on publication flush.


Listeners
=================

*listeners* object describes a collection of subscribers for one Publication_.

.. note:: 
  
  - You can use the **pairs()** LUA_ function to iterate on the list of Listener_, keys are Client_ objects and values are Listener_ objects.
  - And the "#" operator to get the number of listeners.


Listener
=================

*listener* object describes a subscriber for one publication.

properties
-----------------

- **client** (read-only), refers to the Client_ object which is listening.
- **audioQOS** (read-only), QualityOfService_ object about audio transfer for this subscription.
- **videoQOS** (read-only), QualityOfService_ object about video transfer for this subscription.
- **dataQOS** (read-only), QualityOfService_ object about data transfer for this subscription.
- **publication** (read-only), Publication_ object which describes publication listening by the subscriber.
- **receiveAudio**, boolean to mute audio reception on the subscription.
- **receiveVideo**, boolean to mute video reception on the subscription.


ByteReader
=================

This object is only used for IExternalizable typed object, it's the first argument of *__readExternal* function, and it's an equivalent for IDataInput_ AS3 class (see :ref:`ref-amf-to-lua` to know how AMF/LUA_ conversion works). It contains exactly same functions, excepting *readObject()* which is replaced by *readAMF(x)* function. *readAMF(x)* returns the *x* first LUA_ results which come from the AMF unserialization.


ByteWriter
=================

This object is only used for IExternalizable typed object, it's the first argument of *__writeExternal* function, and it's an equivalent for IDataOutput_ AS3 class (see :ref:`ref-amf-to-lua` to know how AMF/LUA_ conversion works). It contains exactly same functions, excepting _writeObject(object:\*) which is replaced by *writeAMF(...)* function. *writeAMF(...)* takes multiple LUA_ arguments for AMF serialization.


Servers
=================

Servers list of MonaServer currently connected to the server (see :doc:`scalability` page for more details about multiple server features).

properties
-----------------

- **initiators** (read-only), return a Broadcaster_ object including the server initiators. Server connections have a direction, with an iniator of the connection, and a target of the connection (see :doc:`scalability` page for more details). 
- **targets** (read-only), return a Broadcaster_ object including the server targets. Server connections have a direction, with an iniator of the connection, and a target of the connection (see :doc:`scalability` page for more details). 

methods
-----------------

- **broadcast(handler,...)**, broadcast data to servers (initiators and targets). The *handler* parameter is the name of the *remote procedure call* method to receive data, multiple arguments following are the data (see :doc:`scalability` page for more details).
- **(address/index)**, return a Server_ object. It can take the *address* of the server (string format) or the *index* of server (number format) as input argument. Indeed the list is sorted by order of connections.

.. note:: 
  
  - You can use the **ipairs()** LUA_ function to iterate on the list of servers.
  - And the "#" operator to get the number of servers.

.. _ref-server:

Server
===========

*server* object describes a server communication (see :doc:`scalability` page for more details about multiple server features).

properties
-----------------

- **address** (read-only), object address of the server.
- **host** (read-only), hostname of the server.
- **port** (read-only), port used for the server-to-server connection.
- **isTarget** (read-only), true if the server is a target of current server.
- **configs** (read-only), configuration properties of the server (see :ref:`ref-configurations`).

.. note:: *server* object can have other dynamic properties (as Client_ object) which relates properties used during the server connection (see :ref:`ref-configurations`).

methods
-----------------

- **send(handler[,parameters])**, call a method of the server.
- **reject([message])**, disconnect from the server.

You can add some methods into a **Server** object to create RPC functions availables from other servers (see :doc:`scalability` page for more details).


Broadcaster
==============

Allow to manipulate list of server initiators or targets gotten with *servers.initiators* or *servers.targets* (see Servers_ object).

properties
-----------------

- **count** (read-only), number of servers.

methods
-----------------

- **broadcast(handler,...)**, broadcast data to servers. The *handler* parameter is the name of the *remote procedure call* method to receive data, multiple arguments following are the data (see :doc:`scalability` page for more details).
- **(address/index)**, return a Server_ object. It can take the *address* of the server (string format) or the *index* of server (number format) as input argument. Indeed the list is sorted by order of connections.

.. note:: You can use the **ipairs()** LUA_ function to iterate on the list of servers.

File
=============

*File* object gives some properties of a file in the file system. *File* objects are created on a *mona:listPaths(...)* call (see Mona_ object).

properties
-----------------

- **name** (read-only), name of the file
- **baseName** (read-only), name of the file, without extension
- **parent** (read-only), name of the parent directory
- **extension** (read-only), extension of the file
- **size** (read-only), size of the file
- **lastModified** (read-only), date of last modification (in seconds)
- **isFolder** (read-only), true if the file is a directory
- **value** (read-only), full path of the file

IPAddress
==================

*IPAddress* object gives the properties of an IP address. *IPAddress* objects are created on a *mona:createIPAddress(...)* or *mona:createIPAddressWithDNS(...)* call (see Mona_ object).

properties
-----------------

- **isWildcard** (read-only), true if this address is the wildcard address (all zero)
- **isBroadcast** (read-only), true if this address is the local network broadcast address (255.255.255.255, only IPv4 addresses can be broadcast addresses)
- **isAnyBroadcast** (read-only), true if this address is a broadcast address (only IPv4 addresses can be broadcast addresses)
- **isLoopback** (read-only), true if this address is a loopback address
- **isMulticast** (read-only), true if this address is a multicast address (224.0.0.0 to 239.255.255.255 for IPv4, FFxx:x:x:x:x:x:x:x range for IPv6)
- **isUnicast** (read-only), true if this address is a unicast address (if it is neither a wildcard, broadcast or multicast address)
- **isLinkLocal** (read-only), true if this address is a link local unicast address (169.254.0.0/16 range for IPv4, FE80:: for IPv6)
- **isSiteLocal** (read-only), true if this address is a a site local unicast address (10.0.0.0/24, 192.168.0.0/16 or 172.16.0.0 to 172.31.255.255 ranges for IPv4, FEC0:: for IPv6
- **isIPv4Compatible** (read-only), true if this address is IPv4 compatible (for IPv6 the address must be in the ::x:x range)
- **isIPv4Mapped** (read-only), true if this address is an IPv4 mapped IPv6 address (For IPv6, the address must be in the ::FFFF:x:x range)
- **isWellKnownMC** (read-only), true if this address is a well-known multicast address (224.0.0.0/8 range for IPv4, FF0x:x:x:x:x:x:x:x range for IPv6)
- **isNodeLocalMC** (read-only), true if this address is a node-local multicast address (always false for IPv4, in the FFx1:x:x:x:x:x:x:x range for IPv6)
- **isLinkLocalMC** (read-only), true if this address is a link-local multicast address (224.0.0.0/24 range for IPv4, FFx2:x:x:x:x:x:x:x range for IPv6)
- **isSiteLocalMC** (read-only), true if this address is a site-local multicast address (239.255.0.0/16 range for IPv4, FFx5:x:x:x:x:x:x:x for IPv6)
- **isOrgLocalMC** (read-only), true if this address is an organization-local multicast address (239.192.0.0/16 range for IPv4, FFx8:x:x:x:x:x:x:x range for IPv6)
- **isGlobalMC** (read-only), true if this address is a global multicast address (224.0.1.0 to 238.255.255.255 range for IPv4, FFxF:x:x:x:x:x:x:x range for IPv6)
- **isLocal** (read-only), true if this address is local
- **isIPv6** (read-only), true if this address is a IPv6 address
- **value** (read-only), the string representation of the address

SocketAddress
==================

*SocketAddress* object represents a pair *host:port* of a socket connection. *SocketAddress* objects are created on a *mona:createSocketAddress(...)* or *mona:createSocketAddressWithDNS(...)* call (see Mona_ object).

properties
-----------------

- **host** (read-only), the IPAddress_ object of this socket address
- **port** (read-only), the port of the socket address
- **isIPv6** (read-only), true if the host is an IPv6 address
- **value** (read-only), the string representation of the socket address (*host:port*)

.. _ref-events:

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

Called when the server application is built and executed the first time. The first argument is the *path* of the application (see :ref:`ref-create-server-app`).

.. warning:: All server application are built on first client connection for the application, except *root* application (*/* application), which is started on MonaServer starting.


onStop(path)
=====================

Called when the server application is unloaded. It happens in three different cases:

- When you edit *main.lua* file of one server application. Application is restarted (stopped and started).
- When you delete a server application.
- When MonaServer is stopping.

The first argument is the *path* of the application (see :ref:`ref-create-server-app`).


onConnection(client,...)
=============================

Called on a new client connection. First argument is a Client_ object, and following arguments depend on the protocol (see :doc:`protocols`).

Finally you can return a table result to send some informations on RTMP&RTMFP connections (see :doc:`protocols`) or to overload some configuration parameters:

- **timeout** , timeout in seconds. It overloads the timeout parameter from the configuration file (see :doc:`installation`).

.. code-block:: lua

  function onConnection(client,...)
    return {message="welcome",id=1,timeout=7}
  end

The as3 code below illustrates the returned parameters on RTMP&RTMFP connections:

.. code-block:: as3

  function onStatusEvent(event:NetStatusEvent):void {
    switch(event.info.code) {
      case "NetConnection.Connect.Success":
      trace(event.info.message); // displays "welcome"
      trace(event.info.id); // displays "1"
      break;
    }
  }

You can reject a client by adding an error of connection:

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

In RTMP&RTFMP it answers with a *NetConnection.Connect.Rejected* status event and close the client connection. The *event.info.description* field contains your error message. Now if you reject a client with no error message, *event.info.description* field will contain "client rejected" by default.

You can subscribe for Client_ events under the onConnection scope like in this sample :

.. code-block:: lua

  function onConnection(client)
    
    function client:onMessage(message)
      NOTE(client.address.." says "..message)
    end
	
  end


onDisconnection(client)
============================

Call on Client_ disconnection. *client* argument is the disconnected client.

.. note:: At this stage you can send no more data to the client, all writing to a Writer_ object of this client has no effect.


.. _ref-onManage:

onManage()
=====================================

Call every two seconds, this event is available only in the *root* server application (*www/main.lua*). It allows easyly to get handle to manage your objects if need.

.. _ref-onRendezVousUnknown:

onRendezVousUnknown(protocol, peerId)
=====================================

Allows to redirect a client who searchs a peerId that the rendezvous service doesn't find. Usually you will redirect the client to one or multiple other MonaServer (see :doc:`scalability` for more details on multiple servers usage). You can return an address, but also multiple address, or an array of addresses.

.. code-block:: lua

  function onRendezVousUnknown(protocol, peerId)
    return 192.168.0.2:1935
  end

.. code-block:: lua

  function onRendezVousUnknown(protocol, peerId)
    return 192.168.0.2:1935,192.168.0.3:1935
  end

.. code-block:: lua

  addresses = {192.168.0.2:1936,192.168.0.3:1936}
  function onRendezVousUnknown(protocol, peerId)
    return addresses
  end

Then you can return a Server_ object or a Servers_ object:

.. code-block:: lua
  
  function onRendezVousUnknown(protocol, peerId)
    return mona.servers[1] -- redirect to the first server connected
  end

.. code-block:: lua

  function onRendezVousUnknown(protocol, peerId)
    return mona.servers -- redirect to all the connected servers
  end

.. note:: When this function returns multiple addresses, the client will receive all these addresses and will start multiple attempt in parallel to these servers.

.. _ref-onHandshake:

onHandshake(address,path,properties,attempts)
===============================================

Allows to redirect the client to one other MonaServer (see :doc:`scalability` for more details on multiple servers usage), in returning address(es) of redirection. About the returned value it works exactly same the returned value of :ref:`ref-onRendezVousUnknown` event.
It's called on the first packet received from one client (before the creation of its client object associated). First *address* argument is the address of the client, *path* argument indicates the path expression of connection, *properties* argument is a table with the HTTP parameters given in the URL of connection (see dynamic properties of Client_ object description) and *attempts* argument indicates the number of attempts of connection (starts to 1 and is incremented on each attempt).

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

Call on server connection, see :doc:`scalability` for more details on multiple servers usage, or Server_ object.


onServerDisconnection(server)
====================================

Call on server disconnection, see :doc:`scalability` for more details on multiple servers usage, or Server_ object.


.. _`Query string`: http://en.wikipedia.org/wiki/Query_string
.. _XML-RPC : http://xmlrpc.scripting.com/spec.html
.. _LUA: http://www.lua.org/
.. _NetGroup: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/net/NetGroup.html
.. _IDataOutput: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IDataOutput.html
.. _IDataInput: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IDataInput.html
