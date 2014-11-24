
Installation
###################################

We would like to remind you that Mona is licensed under the `GNU General Public License`_, so **MonaBase can't be linked with any closed source project.**.

.. contents:: Table of Contents

Binaries
***********************************

.. TODO
.. A `32-bit Windows binary <http://jazzmatazz.free.fr/Mona/MonaServer.zip>`_ has been provided to quickly test MonaServer. We recommend
.. you compile a Linux version from the sources for production use. If you download the Windows 32-bit binary, then you can skip down to the
.. *Configurations* section of this document.

Download
***********************************

You can download the entire sources with `zipball <https://github.com/MonaSolutions/MonaServer/zipball/master>`_ or by using a GIT_ Client with the following address: https://github.com/MonaSolutions/MonaServer.git.

Build
***********************************

The Mona source code is cross-platform.

You **must** build Mona in the following order:

#. :  **MonaBase**, 
#. :  **MonaCore**, 
#. :  **MonaServer**.

Dependencies and requirements
===================================

Mona has the following dependencies :
 - OpenSSL_ is required.
 - LuaJIT_ is required.

.. note:: LuaJIT_ is an alternative to the official LUA interpreter. It works faster than LUA essentially because it compiles the LUA code to machine code during execution (see `LuaJIT performance <http://luajit.org/performance_x86.html>`_ about performance comparison).

As **C++11** is used in MonaServer you need to install a compiler which is compatible. At least **gcc 4.8.2** on Linux and VS 2013 on Windows.

LuaJIT installation
===================================

For several reasons some functionalities of **LUA 52** are not allowed by default in luajit (like **#** or **pairs()**) so we recommend you to compile luajit by yourself and set the DLUAJIT_ENABLE_LUA52COMPAT option.

- First download the latest release of LuaJIT_ (here with git) :
    .. code-block:: sh

        git clone http://luajit.org/git/luajit-2.0.git
- Then set the DLUAJIT_ENABLE_LUA52COMPAT option :
    + On Linux just search DLUAJIT_ENABLE_LUA52COMPAT in *src/Makefile* and uncomment the line
    + On Windows search the line beginning with *@set LJCOMPILE=cl /nologo /c /O2 /W3 /D_CRT_SECURE_NO_DEPRECATE* and add the */DLUAJIT_ENABLE_LUA52COMPAT* option
- Finally you can compile luajit (*make* or *src/msvcbuild.bat*)
 
.. note:: On Linux just run **sudo make install** to install luajit on the system.

Windows Build
===================================

First follow the `LuaJIT installation`_ steps.

Visual Studio 2013 solution and project files are included.
It searches external libraries in *External/lib* folder and external includes in *External/include* folder in the root *Mona* folder.
So you must put OpenSSL_ and LuaJIT_ headers and libraries in these folders.
You can find OpenSSL_ binaries for windows on Win32OpenSSL_.

Unix (Linux/OSX) Build
===================================

First follow the `LuaJIT installation`_ steps.

If your Unix system includes a package manager you can quickly install OpenSSL_. The package is usually named *libssl-dev*.

.. warning:: You need to use the *-dev* version to get the header files required during Mona compilation.

To build Mona:

.. code-block:: sh

  cd MonaBase
  make
  cd ../MonaCore
  make
  cd ../MonaServer
  make

To clean:

.. code-block:: sh

  $ cd MonaBase
  $ make clean
  $ cd ../MonaCore
  $ make clean
  $ cd ../MonaServer
  $ make clean

Amazon EC2 AMI Build
===================================

First follow the `LuaJIT installation`_ steps.

Create a standard EC2 AMI with a new security group. Open up all UDP and at least TCP Ports 1935 in the Security Group.

Connect to the AMI and execute the following script:

.. code-block:: sh

  sudo -s
  yum -y groupinstall "Development Tools"
  yum -y groupinstall "Development Libraries"
  yum -y install make
  wget https://github.com/MonaSolutions/MonaServer/archive/master.zip
  unzip Mona-master.zip
  cd Mona-master
  cd MonaBase
  make
  cd ..
  cd MonaCore
  make
  cd ..
  cd MonaServer
  make
  sudo ./MonaServer --daemon

Raspberry PI (armv61) Build
===================================

You need to install a distribution where **gcc 4.8.2** is available. For now we have only found Raspbian_ upgraded to the distribution *jessie*. Here is the protocol to install it correctly :

- First download Raspbian_
- Then write the distribution image on an SD Card (`How to write Raspberry Pi image to SD card <http://xmodulo.com/2013/11/write-raspberry-pi-image-sd-card.html>`_)
- Start the raspberry pi and install raspbian
- Update the /etc/apt/sources.list to have at least *jessie* (rather than *wheezy*)
- Run theses steps to upgrade Raspbian_ :

.. code-block:: sh

    sudo aptitude update
    sudo aptitude dist-upgrade
    sudo rpi-update
    reboot

- Follow the `LuaJIT installation`_ steps
- And then run `Unix (Linux/OSX) Build`_ steps

Configurations
***********************************

MonaServer is statically configured by an optional configuration *MonaServer.ini* file to put in the installation folder.
Possible configurations are :

- **host** : address like it will be seen by clients and other servers, this option is mandatory to make working all redirection features in multiple server configuration (see `Scalability and load-balancing <./scalability.html>`_).
- **socketBufferSize** : allows to change the size in bytes of sockets reception and sending buffer. Increases this value if your operating system has a default value too lower for important loads.
- **threads** : indicates the number of threads which will be allocated in the pool of threads of Mona. Usually it have to be equal to (or greather than) the number of cores on the host machine (virtual or physic cores). By default, an auto-detection system tries to determinate its value, but it can be perfectible on machine who owns hyper-threading technology, or on some operating systems.
 
[application]
===================================

- **dir** : Directory containing *data* and *www*.
 
[servers]
===================================

- **port** : port to receive incoming server connection (in a multiple servers configuration, see `Scalability and load-balancing <./scalability.html>`_ for more details). If you don't configure this port, MonaServer cannot establish a connection with it.

.. warning::  The exchange between servers is done in a unencrypted TCP way, so to avoid an attack by this incoming end point the *servers.port* should be protected by a firewall to allow just a connection by an other server and nothing else.
 
- **targets** : list of MonaServer addresses (separated by semicolons) to connect on start-up. When the server will start, it will try to etablish a connection to these addresses every 10 seconds (see `Scalability and load-balancing <./scalability.html>`_ for more details). Each token can include arguments through as a query url form:

.. code-block:: ini

  [servers]
  targets=192.168.0.2:1936?name=master&arg=val;192.168.0.3:1936

It will create dynamic properties on *server* object (see *server* object description of "Server application, api" page for more details).

.. code-block:: lua

  function onServerConnection(server)
    if server.name=="master" then -- true here just for 192.168.0.2:1936 server
      NOTE("server master arg = "..server.arg) -- displays here "server master arg = val"
    end
  end

[RTMFP]
===================================

- **port** : equals 1935 by default (RTMFP server default port), it is the port used by MonaServer to listen incoming RTMFP requests.

- **keepaliveServer** : time in seconds for periodically sending packets keep-alive with server, 15s by default (valid value is from 5s to 255s).

- **keepalivePeer** : time in seconds for periodically sending packets keep-alive between peers, 10s by default (valid value is from 5s to 255s).

[RTMP]
===================================

- **port** : equals 1935 by default (RTMP server default port), it is the port used by MonaServer to listen incoming RTMFP requests.

[HTTP]
===================================

- **port** : equals 80 by default (HTTP server default port), it is the port used by MonaServer to listen incoming HTTP requests.
- **timeout** : 7 by default, it is the maximum time before server kills the connection when no data as been received.
- **index** : the default index file of HTTP protocol, if it is specified it will redirect each connection to this index.

[WebSocket]
===================================

- **timeout** : 120 by default, it is the maximum time before server kills the connection when no data as been received.

[logs]
===================================

- **directory** : directory where the log files are written (*MonaServer/logs* by default).
- **name** : name of log files (*log* by default).
- **rotation** : number of files to keep in *logs* directory 
 
.. note:: Maximum size of a file is 1Mb.

Sample of MonaServer.ini
===================================
 
.. code-block:: ini

  ;MonaServer.ini
  socketBufferSize = 114688
  [RTMFP]
  port = 1985
  keepAlivePeer = 10
  keepAliveServer = 15
  [logs]
  name=log
  directory=C:/MonaServer/logs

If this configuration file doesn't exist, default values are used.

Launch
***********************************

Start
===================================

MonaServer includes some argument launch options, but by default MonaServer is optimized for a production running. Command-line options are useful during development and test usage. To get full descriptions about the launch arguments start MonaServer with */help* argument on Windows or *--help* on Unix system.

Otherwise, simply start the MonaServer application with administrative rights.

You can also start it as a Windows service:

.. code-block:: sh

  MonaServer.exe /registerService [/displayName=MonaServer /description="Open Source RTMFP Server" /startup=automatic]

Or an Unix daemon:

.. code-block:: sh

  sudo ./MonaServer --daemon [--pidfile=/var/run/MonaServer.pid]


Usage
===================================

Flash client connects to MonaServer by the classical NetConnection way:

.. code-block:: as3

    _netConnection.connect("rtmfp://localhost/");

Here the port has its default value 1935. If you configure a different port on MonaServer you have to indicate this port in the URL (after *localhost*, of course).

The path used allows you to connect for your desired `Server Application <./serveapp.html>`_.

.. code-block:: as3

    _netConnection.connect("rtmfp://localhost/myApplication");

To learn more, read the `Server Application <./serveapp.html>`_ or `Samples <./samples.html>`_ page.

.. _Win32OpenSSL : http://www.slproweb.com/products/Win32OpenSSL.html
.. _LuaJIT : http://luajit.org/
.. _OpenSSL : http://www.openssl.org/
.. _`GNU General Public License` : http://www.gnu.org/licenses/
.. _GIT : http://en.wikipedia.org/wiki/Git_(software)
.. _Raspbian : http://downloads.raspberrypi.org/raspbian_latest
