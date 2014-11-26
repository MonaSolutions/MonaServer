
FAQ
##############################

.. contents:: Table of Contents

What are the advantages of RTMFP?
*******************************************

**RTMFP (Real Time Media Flow Protocol)** is the most powerful protocol to deliver live real-time communication for these principal reasons:

- RTMFP supports Native IP Multicast, Application Level Multicasting (using P2P), and the combination of both called Fusion Multicasting.
- RTMFP is based on the User Datagram Protocol (UDP), which increases the speed of delivery; Audio and Video packets are not forced to be retransmitted, which greatly reduces latency.
- RTMFP utilizes peer-to-peer (P2P) assisted network communication (end-users communicate directly), which consumes minimal server bandwidth and system resources.

Why doesn't the `Cirrus Sample Application`_ work?
***************************************************

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

.. Note:: The `Cirrus Sample Application`_ requires some python scripts to exchange user names, which can be achieved much easier with LUA code. Take a look at the *Meeting Sample* in `Samples <./samples.html>`_ page which does the same thing, but without other external dependencies.

How to use the as3 Socket class with Mona?
****************************************************

Due to new policy rules it is not possible to use the Socket class online without having a policy file. The easier way to do this is to return the socket policy file on each connection on the port 843 (the default port used by Flash):

.. code-block:: lua

  serverPolicyFile = mona:createTCPServer()
  function serverPolicyFile:onConnection(client)
          
          function client:onData(data)
                  INFO("Sending policy file...")
                  self:send("<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0")
                  return 0 -- return rest (all has been consumed here)
          end
  end
  serverPolicyFile:start(843); -- start the server on the port 843
    
.. Note:: In this sample we give access to each ports and from any domain.

Is there a way to record audio&video stream?
****************************************************

Recording feature is on the roadmap of Mona, until now we have prefered to put that on hold because it miss a async file mechanism in Mona to be able to manipulate files asynchronously like sockets (IOCP for Windows and libaio for linux).
We could implement it in a classic "blocking way" but it will decrease Mona performance, not our goal, we prefer keep full real-time reactivity of Mona. If we find financial resources to develop it (see `Support page <./contacts.html>`), it could become our priority but until nobody has give funds for that (Mona development is our full time job, and this feature requires between 2 weeks and 1 month of job).

How to create a C++ plugin extending lua?
****************************************************

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

.. _`Cirrus Sample Application`: http://labs.adobe.com/technologies/cirrus/samples/
