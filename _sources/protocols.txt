
Specific Protocol Functionalities
###########################################

In MonaServer we have taken the choice to give a powerfull **generic** access to protocols. For this reason we take importance to the choice of protocols implemented in order to fit well with our architecture.

This page treat the specificities of each protocol implemented by MonaServer.

.. contents:: Table of Contents

RTMP & RTMFP
*******************************************

API specificities
===========================================

onConnection returned array
-------------------------------------------

Parameters can be sended to the client on connection by returning them in an associative array. Here is an example :

.. code-block:: lua

  function onConnection(client,...)
    return {message="welcome",id=1}
  end

Some parameters are specific to RTMP :

- **timeout** , timeout in seconds. It overloads the timeout parameter from the configuration file (see `Installation`_).

onConnection parameters
-------------------------------------------

As it is said in the `LUA API`_ first parameter of *onConnection* is the **Client** object.

Others ar AMF parameters given to *NetConnection:connect(address:String, ... parameters)* converted in LUA_ types (see *AMF and LUA types conversion* part of `Server Application`_ page to know how AMF/LUA_ conversion works).

client.properties
-------------------------------------------

In RTMP & RTMFP protocols the client properties are **URL query parameters** and standard informations given by flash client at connection.

Here is a list of known static flash client parameters :

- **swfUrl** (read-only), URL of the SWF file which has gotten the connection.
- **tcUrl** (read-only), RTMPF/RTMFP URL used to connect to this session.
- **pageUrl** (read-only), URL of the page which has gotten the connection, it means the URL of the page which contains the SWF client.
- **flashVer** (read-only), flash version of the client.
- **app** (read-only), The Server application name the client is connected to.
- **fpad** (read-only), True if proxy is being used.
- **audioCodecs** (read-only), Indicates what audio codecs are supported.
- **videoCodecs** (read-only), Indicates what video codecs are supported.
- **objectEnconding** (read-only), AMF encoding method.
- **capabilities** (read-only), 
- **videoFunction** (read-only), the SUPPORT_VID_CLIENT_SEEK constant.

And here is a sample of RTMFP URL query parameters sended by a flash client :

.. code-block:: as3

  _netConnection.connect("rtmfp://localhost/myApplication?arg1=value1&arg2=value2");

HTTP & WebSocket
*******************************************

API specificities
===========================================

onConnection returned array
-------------------------------------------

Some specific parameters can be set returning an associative array like this :

.. code-block:: lua

  function onConnection(client,...)
    return {index="index.html", timeout=7}
  end

Here is the list of the possible common parameters :

- **timeout** , timeout in seconds. It overloads the timeout parameter from the configuration file (see `Installation`_).

And here the HTTP-only parameters :

- **index**, true by default. If *true* the Server will send a view of the application's directory, otherwise return a *404 error* file. If it is a string MonaServer will try to return the corresponding file in the application's directory.

onConnection parameters
-------------------------------------------

As it is said in the `LUA API`_ first parameter of *onConnection* is the **Client** object.

The following parameter is an array which contains **URL query parameters** from the HTTP request.

.. code-block:: lua

    -- show each parameters from url
  function onConnection(client, parameters)
    INFO(mona:toJSON(parameters))
  end

client.properties
-------------------------------------------

With HTTP protocol the client properties maps cookies sended by client plus some connection properties :

- **HTTPVersion** (read-only), the version of HTTP used.

client.properties(key, value[, expires, path, domain, secure, httpOnly])
-------------------------------------------------------------------------

As *client.properties* maps cookies values you can also set cookies by using the *client.properties()* method. Parameters are :

- **key**, the key name of the cookie.
- **value**, a string representing the value of the cookie.
- **expires** (optional), 0 by default, an integer value that represents the number of seconds (since now) that the client should keep the cookie.
- **path** (optional), the server path on which the cookie applies.
- **domain** (optional), the domain on which the cookie should be send.
- **secure** (optional), true if the cookie should be send only on a securised connection.
- **httpOnly** (optional), true if the cookie should be visible only by the HTTP protocol.

The return value is the **value** parameter if the operation succeed.

Here is an example of a cookie named *test* with a value of *value1* that should be sended by client for the 5 next minutes on each application (*"/"*) only for HTTP requests to the host *192.168.0.1* :

.. code-block:: lua

    INFO("test : ", client.properties("test", "value1", 300, "/", "192.168.0.1", false, true))

.. note:: To unset a cookie on the client side you can set a negative value to the **expires** parameter.

.. _Installation: ./installation.html
.. _Server Application: ./serverapp.html
.. _LUA API: ./api.html
.. _LUA: http://www.lua.org/
