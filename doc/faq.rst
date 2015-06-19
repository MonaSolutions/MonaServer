
FAQ
##############################

.. contents:: Table of Contents

Technical questions
********************

Why does connections from IPv6 addresses doesn't work on Windows XP (or 2000)?
===============================================================================

By default Windows XP doesn't support IPv6, you just need to install it in the commande line :

.. code-block:: bash

  netsh int ipv6 install

Why do I have the error "HTTP server, Permission denied..." at start?
======================================================================

It's not an error, you have already a process listening on the port 80, maybe Skype or Apache?
If you don't want to use the HTTP/WebSocket part of MonaServer you can continue without doing anything. Otherwise you can :

- kill the other process listening on the port 80 and restart MonaServer,
- or `Configure the HTTP port`_ of MonaServer in the file MonaServer.ini (create the file)

​
Here is a MonaServer.ini sample :

.. code-block:: ini

  [HTTP]
  port=8081



Why doesn't the `Cirrus Sample Application`_ work?
================================================================

Cirrus Server requires the following script:

.. code-block:: lua

  function onConnection(client,...)
    function client:relay(targetId,...)
      target = mona.clients(targetId)
      if not target then
        error("client '"..targetId.."' not found")
        return
      end
      target.writer:writeInvocation("onRelay",self.id,...)
      target.writer:flush(true)
    end
  end

You can add the script (seen above) in *www/main.lua* to make the `Cirrus Sample Application`_ work. 

Let see **VideoPhone**, our improved version of the Cirrus Sample, or take a look at the **Meeting sample** which combine RTMFP and RTMP and add meeting rooms : :doc:`samples`.

How to use the as3 Socket class with Mona?
===============================================================================

Due to new policy rules it is not possible to use the Socket class online without having a policy file. The easier way to do this is to return the socket policy file on each connection on the port 843 (the default port used by Flash):

.. code-block:: lua

  serverPolicyFile = mona:createTCPServer()
  function serverPolicyFile:onConnection(client)
          
          function client:onData(data)
                  INFO("Sending policy file...")
                  self:send("<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0")
                  return data.length -- (all has been consumed here)
          end
  end
  serverPolicyFile:start(843); -- start the server on the port 843
    
.. Note:: In this sample we give access to each ports and from any domain.

Is there a way to record audio&video stream?
===============================================================================

Recording feature is on the roadmap of Mona, until now we have prefered to put that on hold because it miss an async file mechanism in Mona to be able to manipulate files asynchronously like sockets (IOCP for Windows and libaio for linux).
We could implement it in a classic "blocking way" but it will decrease Mona performance, not our goal, we prefer keep full real-time reactivity of Mona. If we find financial resources to develop it (see :doc:`contacts`), it could become our priority but until nobody has give funds for that (Mona development is our full time job, and this feature requires between 2 weeks and 1 month of job) it could take a lot of time...

How to create a C++ plugin extending lua?
===============================================================================

It is very easy to create a new library extending your lua functionalities. For example the following c++ source code implements a *printtest(message)* function:

.. code-block:: c++

  // Don't forget extern "C"!
  extern "C" {
  #include "luajit-2.0/lauxlib.h"
  }

  // The function printtest implementation in C++
  int lua_printtest(lua_State* L)
  {
    const char* message = luaL_checkstring(L, 1);
    printf("printtest : %s\n", message);
    return 0;
  }
  
  extern "C" __declspec(dllexport) int luaopen_LibLua (lua_State* L)
  {
    lua_register(L, "printtest",  lua_printtest);
    return 0;
  }

Now just compile the project and put the library in the execution directory of MonaServer. Restart Mona. That's all!

.. Note:: Don't forget to link with luajit library and include files.


General questions
********************

What are the advantages of RTMFP?
====================================

**RTMFP (Real Time Media Flow Protocol)** is the most powerful protocol to deliver live real-time communication for these principal reasons:

- RTMFP supports Native IP Multicast, Application Level Multicasting (using P2P), and the combination of both called Fusion Multicasting.
- RTMFP is based on the User Datagram Protocol (UDP), which increases the speed of delivery; Audio and Video packets are not forced to be retransmitted, which greatly reduces latency.
- RTMFP utilizes peer-to-peer (P2P) assisted network communication (end-users communicate directly), which consumes minimal server bandwidth and system resources.

What can I do with MonaServer?
===============================================================================

First take a look at our :doc:`Main page <index>` which describe briefly the scope of MonaServer. Then we could take some samples :

- Videoconference (please take a look at the :doc:`Meeting sample <samples>`) or a chat service,
- Remote desktop control or sharing screen online applications (see the `OBS Guide`_ on how to configure the great projet OBS_ with MonaServer),
- Mutiplayer online P2P games like Haxball_,
- Industrial Computing with web interface (using our :doc:`socket classes <serversocket>` or `rpi-gpio`_ for example on a Raspberry pi),
- API Server using HTTP JSON or XML-RPC (to serve other formats, please :doc:`ask us <contacts>`),
- WebTV channels using RTMFP and *mona:publish()* on server side to publish from an external source,
- Online P2P sharing file site,
- Dynamic web applications communicating over any of our protocols (WebSocket, RTMFP, RTMP or HTTP),
- Etc... Everything is possible as MonaServer is fast, scalable and oriented for web applications.

What are the differences between CumulusServer and MonaServer?
==========================================================================

Cumulus_ is made obsolete by **MonaServer** because they are too much differences to maintain the two repositories.

Here is a list of main features added in **MonaServer** :

- No more Poco dependency resulting in a lightweight project,
- A complete C++11 framework for implementing new protocols,
- New protocols like RTMP/RTMPE, HTTP, Websocket and RTSP (this one is in beta version),
- Any RPC function is accessible by each protocol,
- Many bug fixes,
- A best architecture and optimized performances,
- A :doc:`NoSQL cache system <database>` for persistence of data,

And we expect to add more features in the future but it depends on your investments.

.. _`Cirrus Sample Application`: http://labs.adobe.com/technologies/cirrus/samples/
.. _Haxball : http://www.haxball.com
.. _OBS : https://obsproject.com
.. _`OBS Guide` : https://obsproject.com/forum/resources/how-to-set-up-your-own-private-rtmfp-server-using-monaserver.153/
.. _`rpi-gpio` : https://github.com/Tieske/rpi-gpio/tree/master/lua
.. _`Configure the HTTP port` : http://www.monaserver.ovh/installation.html#http
.. _Cumulus : https://github.com/OpenRTMFP/Cumulus/
