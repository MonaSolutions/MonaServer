
.. image:: githubBlack.png
  :align: right
  :target: https://github.com/MonaSolutions/MonaServer

Server Application Sockets
##############################

`Server Application <./serveapp.html>`_ in MonaServer uses LUA_ engine. This engine can be extended with some LUA_ extensions to add some required features (for more details see `Server Application <./serveapp.html>`_ page.)
Usually to add sockets feature in LUA_, the extension LUASocket_ is used. But this extension has three disavantages to create `Server Application <./serveapp.html>`_ in MonaServer:

- All methods to receive and send data are in a blocking mode by default. It can be changed, but it works not correctly especially in TCP. LUA_ is essentialy single-threaded, and the handle have to return to Mona quickly on each script processing. We have need of non-blocking socket features which works with performance in all circumstances.
- LUASocket_ don't work always fine on Windows.
- It duplicates uselessly socket cross-platform features already provided by Mona in its core.

For all these reason, I am making available a new LUA_ socket features mapping on Mona socket intern framework, with only non-blocking methods.

.. contents:: Table of Contents

TCP Client
********************************

To create a TCP client, call *mona:createTCPClient()* method (see *Mona* object description on `Server Application, API <./api.html>`_ page).
Here a complete sample to understand its usage:

.. code-block:: lua

	socket = mona:createTCPClient()
	function socket:onReception(data)
		NOTE("Reception from "..self.peerAddress.." to "..self.address)
		self:send(data) -- echo sample
		return 0 -- return rest (all has been consumed here)
	end
	function socket:onDisconnection()
		if self.error then -- error? or normal disconnection?
			ERROR(self.error)
		end
		NOTE("TCP disconnection")
	end

	local err = socket:connect("localhost",1234)
	if err then ERROR(err) end
	...
	if socket.connected then -- useless if already disconnected
		socket:disconnect()
	end


properties
=============================

- **connected** (read-only), return *true* if the client is connected, otherwise return *false*.
- **address** (read-only), address local of connection for this TCP socket.
- **peerAddress** (read-only), peer address of connection for this TCP socket.
- **error** (read-only), return the last error or *nil* if no error.


methods
=============================

- **connect(host,port)**, connect to the *host:port* indicated. If this method fails, it returns an error message, otherwise it returns nothing.
- **disconnect()**, shutdown the socket.
- **send(data)**, send *data* (LUA_ string).


events
=============================

- **onReception(data)**, call on data reception (data is a LUA_ string). Have to return the number of bytes remaining, it means that if you return *#data* (size of data received), on the next reception, *data* will contain precedent value concatenated with new data received.
- **onDisconnection()**, call on socket disconnection.



TCP Server
********************************

To create a TCP server, call *mona:createTCPServer()* method (see *Mona* object description on `Server Application, API <./api.html>`_ page).
Here a complete sample to understand its usage:

.. code-block:: lua

	server = mona:createTCPServer()
	function server:clientHandler(client)
		-- Here we have a TCPClient object, same usage than TCPClient
		function client:onReception(data)
			NOTE("Reception from "..self.peerAddress.." to "..self.address)
			self:send(data) -- echo sample
			return 0 -- return rest (all has been consumed here)
		end
		function client:onDisconnection()
			NOTE("TCP client disconnection")
		end
	end
	server:start(1234); -- start the server on the port 1234

properties
=============================

- **port** (read-only), return the listening port for the TCP server.
- **running** (read-only), return *true* if the TCP server is running.


methods
=============================

- **start(port)**, start the TCP server on the port given. This method returns *true* if successful, otherwise it returns *false* and displays a *ERROR* log in MonaServer logs.
- **stop()**, stop the TCP server.


events
=============================

- **clientHandler(client)**, call on client connection. Client parameter is a TCP client as described in the precedent *TCP Client* part (see above).


Mails feature
********************************

Actually, it boils down to send an email from server application script code.
In first, you have to configure a SMTP server in MonaServer configurations (see `Installation <./installation.html>`_ page for a complete description of these configurations).

.. code-block:: ini

	;MonaServer.ini
	[smtp]
	host=smtp.isp.com
	port=25
	timeout=60
 
Then you have to use *mona.sendMail(sender,subject,content,...)* method to send an email from *sender* to recipients given in the last mutiple arguments field (see *Mona* object description on `Server Application, API <./api.html>`_ page). It returns a mail object which contains only one event, *onSent(error)*. You can use this event to get one notification on sent, and if *error* is not null it means that the send has failed.

Here a simple sample:

.. code-block:: lua

	mail = mona:sendMail("test@domain","Hello","Mail sent from script code","test@domain")
	function mail:onSent(err)
		if err then ERROR(err) else NOTE("Sent") end
	end



UDP Socket
********************************

To create a UDP socket, call *mona:createUDPSocket([allowBroadcast])* method (see *Mona* object description on `Server Application, API <./api.html>`_ page).
Here a echo sample to understand its usage:

.. code-block:: lua

	socket = mona:createUDPSocket()
	function socket:onReception(data,address)
		NOTE("Reception from "..address)
		self:send(data,address) -- echo sample
	end
	err = socket:bind("0.0.0.0:1234") -- start the server
	if error then ERROR(err) end

Following a sample in a client form, in connected mode:

.. code-block:: lua

	socket = mona:createUDPSocket()
	function socket:onReception(data,address)
		NOTE("Reception from "..address..": "..data)
	end
	socket:connect("0.0.0.0:1234")
	NOTE("UDP socket opened on ",socket.address," connected to ",socket.peerAddress)
	socket:send("salut")


properties
=============================

- **address** (read-only), address local of connection for this UDP socket (returns NULL in an unconnected socket mode)
- **peerAddress** (read-only), peer address of connection for this UDP socket (returns NULL in an unconnected socket mode)

methods
=============================

- **connect(address)**, connect to the *address* indicated. Then UDP packets can be sent without using *address* argument in *send* method (see below).
- **bind(address)**, bind to the *address* indicated. It can not be done on a connected socket. If this method fails, it returns an error message, otherwise it returns nothing.
- **send(data[,address])**, send *data* (LUA_ string) to the *address* indicated. This *address* argument can be omitted if the UDP socket is in a connected mode (see *connect* method above).
- **close()**, close the socket.

events
=============================

- **onReception(data,address)**, call on data reception (data is a LUA_ string). The *address* argument is the sender.


Other protocols
********************************

All is possible in a non-blocking mode, and without using LUASocket_ extension, contact <mathieu.poux@gmail.com> or <jammetthomas@gmail.com> for help.

.. _LUA: http://www.lua.org/
.. _LUASocket: http://w3.impa.br/~diego/software/luasocket/
