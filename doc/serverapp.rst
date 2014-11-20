
Server Application
##############################

MonaServer includes a powerfull script-engine (using LUA_ langage) to create your own server applications. It allows to extend Mona behavior to add your specific needs in a flexible scripting-way.

.. contents:: Table of Contents

Create a server application
******************************

On start, Mona creates a *www* folder in its root directory if it doesn't exist already. In this space, you can create subfolders, each one of them describes one server application. When a client connects to MonaServer, the URL path relates the application to use.
For example, the address *rtmfp://host:port/myApplication* search its corresponding application in *MonaServer/www/myApplication* folder. First time MonaServer builds and executes the file *main.lua* presents in this folder. Then, on new comer, the application already built is executed, unless *main.lua* file has been modified since the last time. Indeed when you edit *main.lua* file, the corresponding application is rebuilt in a dynamic way, without having to restart the server.

.. code-block:: text

  rtmfp://host:port/                   ->     MonaServer/www/main.lua (root application)
  rtmfp://host:port/myApplication      ->     MonaServer/www/myApplication/main.lua
  rtmfp://host:port/Games/myGame       ->     MonaServer/www/Games/myGame/main.lua

.. note::

	- The root application is built and started on MonaServer start, whereas other server applications are started on the first client connection.
	- Exceptionaly you can give root rights to a child application with the function **children()** as shown below. It permits to start other applications at start.

.. code-block:: lua

	children("childapp")

Each application is identified by its *path*, which is exactly the *path* part of the RTMFP URL connection. In the example above, *root application* has an empty string for path, then the both others have respectively */myApplication* and */Games/myGame* for *path* values.

Here a very simple first server application:

.. code-block:: lua

  function onStart(path)
    print("Server application '"..path.."' started")
  end
  function onStop(path)
    print("Server application '"..path.."' stopped")
  end


Global configurations
******************************

*MonaServer.ini* file allows to give some global configuration values on MonaServer start-up (see `Installation <./installation.html>`_ page). To access for these values from script, use *mona.configs* property (see *Mona* object description in `Server Application, API <./api.html>`_ page to get more details). This system allows to create your own global configuration values and access to them in scripts.

.. code-block:: ini

  ; MonaServer.ini file
  port=19350 ; existing config
  ; New custom config
  [myGroup]
  myConfig=test


.. code-block:: lua

  -- A script file
  print(mona.configs.port) -- displays "19350"
  print(mona.configs.myGroup.myConfig) -- displays "test"

Communication between server applications
********************************************

One application can access (read or write) to a variable of one other application, and can call one function of one other application. But it goes much further than that, applications can be specialized, and inherit them, exactly like inheritance of classes.

Application inheritance
============================================

Principle is simple, for example the */Games/myGame* application extends the */Games* application, and so all functions and variables available in */Games/main.lua* are available in */Games/myGame/main.lua*.

.. code-block:: lua

  -- /Games script application
  test = "I am Games"

  
.. code-block:: lua

  -- /Games/myGame script application
  print(test) -- displays "I am Games"

You can overload an inherited variable or an inherited function, and even dynamically remove the overload if need in putting value to nil.

.. code-block:: lua

  -- /Games/myGame script application
  print(test)          -- displays "I am Games"
  test = "I am myGame" -- overloads test variable
  print(test)          -- displays "I am myGame"
  test = nil           -- remove overloading test variable
  print(test)          -- displays "I am Games"

On variable overloading (or function overloading), you can always access for the parent version in prefixing with the parent application name.

.. code-block:: lua

  -- /Games/myGame script application
  print(test)          -- displays "I am Games"
  test = "I am myGame" -- overloads test variable
  print(test)          -- displays "I am myGame"
  print(Games.test)    -- displays "I am Games"

.. note:: The root server application has for path an empty string.

.. code-block:: lua

  -- '/' script application (the root server application)
  function hello()
    print("I am the root application")
  end

.. code-block:: lua

  -- /Games script application
  function hello()
    print("I am /Games application")
  end
  hello() -- displays "I am /Games application"

.. warning:: Events are functions called by the system (see *Events* part of `Server Application, API <./api.html>`_ page), if an application doesn't define *onConnection* event for example, on new client connection for this application, it's the parent application which will receive the event. To avoid it, you have to overload the event in child application, and you can call also the parent version if needed.

.. note:: The keyword *super* is supported to refer to the the parent application:

.. code-block:: lua

  -- /Games script application
  function onConnection(client,...)
    return super:onConnection(client,...)
  end
  super:hello() -- displays "I am the root application"

You can use *client.path* property to check if it's a client connected for this application or for one child application (see `Server Application, API <./api.html>`_ page for more details on *client* object description).


Exchange between unrelated server applications
=================================================

In class inheritance, parent class has no knowledge of its children. However, here a parent server application can access for an child variable or function in checking before its existence.
For example if */Games* application would like to call a *load* function in */Games/myGame* application, it have to check *myGame* existence, if *myGame* returns nil, it means that *myGame* doesn't exist or is not yet started.

.. code-block:: lua

  -- /Games script application
  if myGame then myGame:load() end

.. code-block:: lua

  -- /Games/myGame script application
  function load() end
    ...
  end

By the same way, any applications can do the same thing with any other applications, even without hierarchical relationship.

.. code-block:: lua

  -- /myApplication script application
  if Games then
    if Games.myGame then Games.myGame:load() end
  end

.. code-block:: lua

  -- /Games/myGame script application
  function load() end
    ...
  end

Communication between server and client
*******************************************

Pull data, Remote Procedure Call
===========================================

You have to define your RPC functions as a member of *client* object gotten on connection, its signature will be exactly the same on client and server side. It can take multiple parameters, and it can return one result.

.. code-block:: lua

  function onConnection(client,...)
    function client:test(name,firstname)
      return "Hello "..firstname.." "..name
    end
  end

**Flash client :**

.. code-block:: as3 

  _netConnection.client = this
  _netConnection.call("test",new Responder(onResult,onStatus),"François-Marie","Arouet")

  function close():void { _netConnection.close() }
  function onStatus(status:Object):void {
    trace(status.description)
  }
  
  function onResult(response:Object):void {
    trace(response) // displays "Hello François-Marie Arouet"
  }

.. warning:: When you change default client of NetConnection, the new client should have a *close()* method which closes the connection, because a RTMFP Server can call this function in some special cases

Note that returned result of the scripting function is a writing shortcut for:

.. code-block:: lua

  function client:test(name,firstname)
    client.writer:writeMessage("Hello "..firstname.." "..name)
  end

They both make exactly the same thing.

If the function is not available on the *client* object, it returns a *NetConnection.Call.Failed* status event with *Method 'test' not found* in description field. But you can also customize your own error event:

.. code-block:: lua

  function client:test(name,firstname)
    if not firstname then error("test function takes two arguments") end
    return "Hello "..firstname.." "..name
  end

.. code-block:: as3

  _netConnection.client = this
  _netConnection.call("test",new Responder(onResult,onStatus),"François-Marie");

  function close():void { _netConnection.close() }
  function onStatus(status:Object):void {
    trace(status.description) // displays "..main.lua:3: test function takes two arguments"
  }
  function onResult(response:Object):void {
    trace(response)
  }

Remote Procedure Call with Websocket or HTTP
-----------------------------------------------

Websocket supports JSON RPC and HTTP supports either JSON and XML-RPC_ using the 'Content-Type' header. Here are samples using the same lua server part :

**Websocket client :**

.. code-block:: js

  socket = new WebSocket(host);
  socket.onmessage = onMessage;
  var data = ["test", "François-Marie", "Arouet"];
  socket.send(JSON.stringify(data));
   
  function onMessage(msg){ 
    var response = JSON.parse(msg.data);
    alert(response);
  }
  
**HTTP JSON-RPC client :**

.. code-block:: js

  var xmlhttp = new XMLHttpRequest();
  xmlhttp.open('POST', "", true);
  
  // Manage the response
  xmlhttp.onreadystatechange = function () {
    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
      var response = JSON.parse(xmlhttp.response);
      alert(xmlhttp.response);
    }
  }
  // Send the POST request
  xmlhttp.setRequestHeader('Content-Type', 'application/json');
  var data = ["test", "François-Marie", "Arouet"];
  xmlhttp.send(JSON.stringify(data));
  
**HTTP XML-RPC client :**

.. code-block:: js

  var xmlhttp = new XMLHttpRequest();
  xmlhttp.open('POST', "", true);
  
  // Manage the response
  xmlhttp.onreadystatechange = function () {
    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
      var response = xmlhttp.response;
      alert(response);
    }
  }
  // Send the POST request
  xmlhttp.setRequestHeader('Content-Type', 'text/xml');
  xmlhttp.send("<methodCall><methodName>test</methodName><params><param><value><string>François-Marie</string></value></param><param><value><string>Arouet</string></value></param></params></methodCall>");
  
.. Note:: Here we use the XML-RPC_ format which is fully supported by Mona.

See more samples on `Samples <./samples.html>`_ page.
  
Push data
=======================================

Push data mechanism is either possible with Websocket and Flash using *client.writer* object.

.. code-block:: lua

  client.writer:writeInvocation("onPushData","Rambo","John")

**Flash client :**
  
.. code-block:: as3

  function onPushData(name:String,firstName:String):void {
  }

**Websocket client :**
  
.. code-block:: js

  socket.onmessage = onMessage;
  ...
  function onMessage(msg){
    var response = JSON.parse(msg.data);
    if (response[0] == "onPushData") {
      name = response[1]
      firstName = response[2]
      ...
    }
  }

.. note:: Push data is not possible with HTTP protocol because it is an old protocol based on pull data only. Long polling is a solution for this but is not implemented yet.

Here an example of push data every two seconds (see *Events* part of `Server Application, API <./api.html>`_ page for *onManage* event description):

.. code-block:: lua

  writers = {}
  function onConnection(client,...)
    writers[client] = client.writer
  end
  function onDisconnection(client)
    writers[client] = nil
  end
  function onManage()
    for client,writer in pairs(writers) do
      writer:writeInvocation("refresh")
    end
  end

.. code-block:: as3

  function refresh():void {...}

*client.writer* returns the main flowWriter of this client. A FlowWriter is an unidirectional communication pipe, which allows to write message in a fifo for the client. Each flowWriter has some statistic exchange informations.
When you want push a constant flow with a large amount of data, or if you want to get independant exchange statistics without disrupting the main flowWriter of one client, you can create your own flowWriter channel to push data:

.. code-block:: lua

  writers = {}
  function onConnection(client,...)
    writers[client] = client.writer:newFlowWriter()
  end
  function onDisconnection(client)
    writers[client] = nil
  end
  function onManage()
    for client,writer in pairs(writers) do
      writer:writeInvocation("refresh")
    end
  end

.. code-block:: as3

  function refresh():void {...}

When you create your own flowWriter, you can overload its *onManage* function, allowing you to write the same thing in a more elegant way, which avoid here *writers* table usage, and make the code really more short (see *Objects* part of `Server Application, API <./api.html>`_ page for more details).

.. code-block:: lua

  function onConnection(client,...)
    writer = client.writer:newFlowWriter()
    function writer:onManage()
      self:writeInvocation("refresh")
    end
  end

.. code-block:: as3

  function refresh():void {...}

If you have need of pushing rate greater than two seconds, use *onRealTime* event of root application (see *Events* part of `Server Application, API <./api.html>`_ page for more details).

LUA types conversions
*****************************************

Several types are supported for messages received by server or sended to clients :
 - AMF (for flash clients),
 - JSON,
 - XML-RPC_,
 - XML (by the fromXML_ parser),
 - and raw data (obviously it does not needs conversion).

AMF to LUA conversions
=========================================

Primitive conversion types are easy and intuitive (Number, Boolean, String). Except these primitive types, in LUA_ all is table. Concerning AMF complex type conversions, things go as following:

.. code-block:: lua

  -- LUA table formatted in Object          // AMF Object 
  {x=10,y=10,width=100,height=100}          {x:10,y:10,width:100,height:100}          

  -- LUA table formatted in Array           // AMF Array
  {10,10,100,100}                           new Array(10,10,100,100)

  -- LUA table mixed                       // AMF Array associative
  {x=10,y=10,100,100}                      var mixed:Array = new Array(10,10,100,100);
                                            mixed["x"] = 10;  mixed["y"] = 10;

  -- LUA table formatted in Dictionary     // AMF Dictionary
  {10="test","test"=10,__size=2}           var dic:Dictionary = new Dictionary();
                                            dic[10] = "test";  dic["test"] = 10;

  -- LUA table formatted in ByteArray      // AMF ByteArray
  {__raw="rawdata"}                        var data:ByteArray = new ByteArray();
                                            data.writeUTFBytes("rawdata");

  -- LUA Table formatted in date           // AMF Date
  {year=1998,month=9,day=16,yday=259,      new Date(905989690435)
  wday=4,hour=23,min=48,sec=10,msec=435,
  isdst=false,__time=905989690435}       

On a LUA_ to AMF conversion, priortiy conversion order works as following:

1. If the LUA table given contains the property *__raw*, it's converted to a ByteArray AMF object.
1. If the LUA table given contains the property *__size*, it's converted to a Dictionary AMF object.
1. If the LUA table given contains the property *__time*, it's converted to a Date AMF object.
1. Otherwise it chooses the more adapted conversion (Object, Array, or Array associative).

About *__time* property on a date object, it's the the number of milliseconds elapsed since midnight UTC of January 1 1970 (`Unix time <http://en.wikipedia.org/wiki/Unix_time>`_).

About Dictionary object, LUA_ table supports an `weak keys table <http://www.lua.org/pil/17.html>`_ feature, and it's used in AMF conversion with the *weakKeys* contructor argument of `Dictionary AMF type <http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/Dictionary.html>`_. It means that if you build an AMF Dictionary object with *weakKeys* equals *true* and send it to MonaServer, MonaServer converts it in a LUA_ table with weak keys, and vice versa.

.. note:: Actually Mona supports all AMF0 and AMF3 format, excepts *Vector* and *XML* types.

Custom types
-----------------------------

You can custom your object using typed object feature.
Indeed, when a typed object is unserialized, *onTypedObject* application event is called.

On client side, the AS3 class flag *RemoteClass* have to be added:

.. code-block:: as3

  [RemoteClass(alias="Cat")]
  public class Cat {
    public function Cat () {
    }
    public function meow() {
      trace("meow")
    }
  }

On reception of this type on script-server side, it will call our *onTypedObject* function, and you can custom your object:

.. code-block:: lua

  function onTypedObject(type,object)
    if type=="Cat" then
      function object:meow()
        print("meow")
      end
    end
  end

*object* second argument contains a *__type* property, here equals to *"Cat"* (also equals to the first argument of *typeFactory* function). It means that if you want create a typed object from script-side, and send it to client, you have just to add a *__type* property.

.. code-block:: lua

  function onConnection(client,...)
    response:write({__type="Cat"})
  end

Cient will try to cast it in a *Cat* class.


You can go more further on this principle, and custom the AMF unserialization and serialization in adding __readExternal and __writeExternal function on the concerned object, it relates AS3 object which implements *IExternalizable* on client side (see `IExternalizable <http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IExternalizable.html>`_).
For example, *ArrayCollection* is an externalizable type, and is not supported by default by the conversion system, you can add its support in adding this script code:

.. code-block:: lua

  function onTypedObject(type,object)
    if type=="flex.messaging.io.ArrayCollection" then
      function object:__readExternal(reader)
        self.source = reader:readAMF(1)
      end
      function object:__writeExternal(writer)
        writer:writeAMF(self.source)
      end
    end
  end

*reader* and *writer* arguments are equivalent of `IDataOutput <http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IDataOutput.html>`_ and `IDataInput <http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IDataInput.html>`_) AS3 class (see *Objects* part of `Server Application, API <./api.html>`_ page for more details).


JSON to LUA conversions
=========================================

As in AMF primitive, conversion types are easy and intuitive (Number, Boolean, String). For the rest, things go as following:

.. code-block:: lua

  -- LUA table formatted in Object          // JSON Object 
  {x=10,y=10,width=100,height=100}          {"x":10,"y":10,"width":100,"height":100}

  -- LUA table formatted in Array           // JSON Array
  {10,10,100,100}                           [10,10,100,100]

  -- LUA table mixed                       // JSON Array + Object
  {x=10,y=10,100,100}                      [{"x":10,"y":10},100,100]

.. note::
  
  - Order can differ from original type because there is no attribute order in lua,
  - Notice that in JSON mixed tables don't exist, that's why we must create an Array with an object containing associative values,
  - For serializations reasons JSON data need to be encapsulated in a JSON array '[]'. For example the JSON Array above will be sended by client in this format :
  
.. code-block:: js

  socket.send("[[10,10,100,100]]");

XML data compatibility (XML parser)
=========================================

As mentioned above, Mona traduce XMLRPC calls automatically. For other types of XML data only few LUA_ code lines are needed, using the useful XML parser.

Two methods are available to do this :
 - *fromXML*
 - and *toXML*
 
See *Mona* part of `Server Application, API <./api.html>`_ page for more details.

A sample with SOAP
-----------------------------

Let us begin with an RPC addition method :

.. code-block:: lua

	function onConnection(client,...)
		function client:add(value)
		  return value+1
		end
	end

We can already call this method by an HTTP GET request (the name and the parameters are given in the URI), a JSON POST, XML-RPC or by AMF.
But if we want absolutly to call it from a SOAP client with the following request :

.. code-block:: xml

	<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
		<soap:Body>
			<Add xmlns="http://localhost/">
				<Value>1</Value>
			</Add>
		</soap:Body>
	</soap:Envelope>
	
And the expected response :

.. code-block:: xml

	<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
		<soap:Body>
			<AddResponse xmlns="http://localhost/">
				<AddResult>
					<Value>2</Value>
				</AddResult>
			</AddResponse>
		</soap:Body>
	</soap:Envelope>
	
Just rebuild the LUA_ code lines like this :

.. code-block:: lua

	function onConnection(client,...)
	
		function client:onMessage(data)
			local xml = mona:fromXML(error,data) -- parse the XML data
			
			-- Call the method
			local result = self:add(xml["soap:Envelope"]["soap:Body"].Add.Value.__value)
			
			-- Replace the Add method by AddResult
			local content = xml["soap:Envelope"]["soap:Body"].Add
			content.__name = "AddResponse"
			content.__value = {__name="AddResult",{__name="Value",result}}
			
			-- Rewrite the XML data and send the result
			return mona:toXML(error,xml)
		end
		
		function client:add(value)
			return value+1
		end
	end

XML to LUA conversions
-----------------------------

**XML:**

.. code-block:: xml

	<?xml version="1.0"?>
	<document>
	  <article>
		<p>This is the first paragraph.</p>
		<h2 class='opt'>Title with opt style</h2>
	  </article>
	  <article>
		<p>Some <b>important</b> text.</p>
	  </article>
	</document>

**LUA:**

.. code-block:: lua

	{ xml = {version = 1.0},
		{__name = 'document',
		  {__name = 'article',
			{__name = 'p', 'This is the first paragraph.'},
			{__name = 'h2', class = 'opt', 'Title with opt style'},
		  },
		  {__name = 'article',
			{__name = 'p', 'Some ', {__name = 'b', 'important'}, ' text.'},
		  },
		}
	}


Due to LUA_ language you can access to "This is the first paragraph" in two ways :
1. variable[1][1][1][1]
2. variable.document.article.p.__value


LUA extensions and files inclusion
******************************************

LUA_ can be extended easily, LUA_ extensions can be founded for all needs and on all operating systems. With LUA_ it is a common thing to add some new MonaServer abilities as SQL, TCP sockets, or others.
Install your LUA_ extension library, and add a *require* line for your script. LUA_ will search the extension in some common location related with LUA_ folder installation.

Now if you need to organize your code in different files for your server application, you can use *absolutePath(path)* helpful functions on *Mona* object (see *Objects* part of `Server Application, API <./api.html>`_ page for more details), in addition of *dofile* or *loadfile* LUA_ functions (see `this LUA page <http://www.lua.org/pil/8.html>`_).

.. code-block:: lua

  function onStart(path)
    dofile(mona:absolutePath(path).."start.lua")
  end
  function onConnection(client,...)
    dofile(mona:absolutePath(path).."connection.lua")
  end

.. warning:: If you edit an included file (like *start.lua* or *connection.lua* here), change are not taken as far as *dofile* is not called again or *main.lua* of this server application is not updated again.

LOGS
****************************************

LUA_ *print* function writes text on the output console of MonaServer in a non-formatted way. Also, in service or daemon usage, nothing is printed of course.
The solution is to use logs macros :

- **ERROR**, an error.
- **WARN**, a warning.
- **NOTE**, an important information, displayed by default in Mona log files.
- **INFO**, an information, displayed by default in Mona log files.
- **DEBUG**, displayed only if you start MonaServer with a more high level log (see */help* or *--help* on command-line startup)
- **TRACE**, displayed only if you start MonaServer with a more high level log (see */help* or *--help* on command-line startup)

.. code-block:: lua

  function onStart(path)
    NOTE("Application "..path.." started")
  end
  function onStop(path)
    NOTE("Application "..path.." stopped")
  end

API
****************************************

Complete API is available on `Server Application, API <./api.html>`_ page.

.. _LUA : http://www.lua.org/
.. _XML-RPC : http://xmlrpc.scripting.com/spec.html
