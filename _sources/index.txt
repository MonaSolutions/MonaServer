.. Mona documentation master file

.. image:: img/githubBlack.png
  :align: right
  :target: https://github.com/MonaSolutions/MonaServer

MonaServer Documentation's Guide
#################################

MonaServer starts with the idea that protocols essentially serve the same aims : 
 - pull data (request + response),
 - push data (server -> client or client -> server),
 - read/write file (**VOD** and **RECORDING**),
 - AND communication channel between clients (**P2P** or publish/play **live**).

And with this objective we are proud to give you a generic alternative to existing communication servers with :
 - The powerful **LuaJIT_** compiler combined in a useful lua API to write server applications,
 - An ingenious **NoSQL** database management system,
 - The mighty protocol **RTMFP** which provide P2P channels, UDP reliable and non-reliable communication and many other great features,
 - And all of this developed keeping in mind the 5 following notions: speed, light weight, cross-platform, scalable and elegant **C++11** code.

For now we have already included several protocols :
 - **RTMFP**, **RTMP**, **RTMPE**,
 - **HTTP** (with **JSON-RPC** and **XML-RPC**), **Websocket**.

And we expect to add many others like **WebRTC**, **HLS**, **IPTV**. But remember that our development is open source so contribution is really appreciated (paypal link).

Mona project is under GPL license, please contact us for commercial licence at mathieupoux@gmail.com or jammetthomas@gmail.com.

**Table of Contents**:

.. toctree::
  :maxdepth: 1
  
  samples
  installation
  serverapp
  scalability
  serversocket
  api
  database
  faq
  
.. _LuaJIT : http://luajit.org/
