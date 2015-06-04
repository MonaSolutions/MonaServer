package Other
{
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	import flash.net.NetStream;
	import flash.net.Responder;
	
	public class Publications extends Test
	{
		private var _host:String;
		private var _url:String;
		
		private var _connection1:NetConnection = null;
		private var _connection2:NetConnection = null;
		private var _publication:NetStream = null;
		private var _listener1:NetStream = null;
		private var _listener2:NetStream = null;
		
		private var _listenerCounter:int = 0;		// listener connection counter
		private var _connectionCounter:int = 0;
		private var _failedCounter:int = 0; 		// only for step 3
		private var _step:int = 0; 					// 3 Steps : normal remove of listeners/onUnsubscribe function call/onSubscribe function call returning false
	
		public function Publications(app:FunctionalTests, host:String, url:String) {
			super(app, "Publications", "Test publications and listeners connections/disconnections");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_listenerCounter = 0;
			_connectionCounter = 0;
			_failedCounter = 0;			
			_step = 1;
			initConnection("Publications/");
		}
		
		// Step 1 : Connections
		private function initConnection(publicationApp:String):void {
			_connection1 = new NetConnection();
			_connection1.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection1.connect("rtmfp://" + _host + _url + publicationApp);
			
			_connection2 = new NetConnection();
			_connection2.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection2.connect("rtmfp://" + _host + _url + publicationApp);
		}
		
		// Close all connections properly
		private function terminateConnections(error:String):void {
			
			if (_publication) {
				_publication.removeEventListener(NetStatusEvent.NET_STATUS, onStatusOut);
				_publication.close();
				_publication = null;
			}
			_connection1.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection1.close();
			_connection1 = null;
			_connection2.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection2.close();
			_connection2 = null;
			
			_connectionCounter = 0;
			_listenerCounter = 0;
			
			if (error.length > 0)
				onResult( { err: error } );
			else {
				
				_step++;
				if (_step>3)
					onResult( { } ); // finished!
				else
					initConnection((_step==2)? "Publications/OnUnsubscribe/" : "Publications/OnSubscribe/"); // next step
			}
		}
		
		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					_connectionCounter++;
					// Step 2 : publication creation
					if (_connectionCounter==2) {
						_publication = new NetStream(_connection1);
						_publication.addEventListener(NetStatusEvent.NET_STATUS, onStatusOut);
						_publication.publish("publication");
					}
					break;
				default:
					terminateConnections(event.info.code);
					break;
			}
		}
		
		private function onStatusOut(event:NetStatusEvent):void {
			
			//_app.INFO("onStatusOut : " + event.info.code);
			switch(event.info.code) {
				case "NetStream.Publish.Start":
					// Step 3 : Subscribtions
					_publication.send("message", "test"); // dummy message
					_listener1 = new NetStream(_connection1);
					_listener1.addEventListener(NetStatusEvent.NET_STATUS, onStatusIn,false,0,true);
					_listener1.play("publication");
					
					_listener2 = new NetStream(_connection2);
					_listener2.addEventListener(NetStatusEvent.NET_STATUS, onStatusIn,false,0,true);
					_listener2.play("publication");
					break;
				default:
					break;
			}
		}
		
		
		private function onStatusIn(event:NetStatusEvent):void {
			
			//_app.INFO("onStatusIn : " + event.info.code + " (listener "+((event.target==_listener1)?"1":"2")+")");
			switch(event.info.code) {
					case "NetStream.Play.Start":
						_listenerCounter++;
						// Step 4 : check objects (2 listeners)
						if (_listenerCounter == 2) {
							_listenerCounter = 0;
						 	_connection1.call("getPublications", new Responder(onPublications1,terminateConnections));
						}
						break;
					case "NetStream.Play.Stop":
						_listenerCounter++;
						// Step 6 : check objects (0 listeners)
						if (_listenerCounter == 2) {
						 	_connection1.call("getPublications", new Responder(onPublications2,terminateConnections));
						}
						break;
					case "NetStream.Play.Failed":
						if (_step == 3) {
							_failedCounter++;
							if (_failedCounter==2)
								terminateConnections("");
						} else
							terminateConnections(event.info.code);
						break;
				default:
					break;
			}
		}
		
		// Callback functions
		private function onPublications1(result:Object):void {
			
			if (result.publications.length == 1 && result.publications[0] == "publication") {
				
				if (result.listeners.length == 2) {
					if ((result.listeners[0] == _connection1.nearID && result.listeners[1] == _connection2.nearID) ||
						(result.listeners[0] == _connection2.nearID && result.listeners[1] == _connection1.nearID)) {
						// Step 5 : disconnect listeners
						_listener1.close();
						_listener2.close();
					} else
						terminateConnections("Error in getPublications1, incorrect list of listeners");
					
				} else 
					terminateConnections("Error in getPublications1, incorrect number of listeners : "+result.listeners.length);
			}
			else
				terminateConnections("Error in getPublications1, incorrect list of publications : "+result.publications.length);
		}
		
		
		private function onPublications2(result:Object):void {
			
			if (result.publications.length == 1 && result.publications[0] == "publication") {
				if (result.listeners.length == 0)
					terminateConnections("");
				else 
					terminateConnections("Error in getPublications2, incorrect number of listeners : "+result.listeners.length);
			}
			else
				terminateConnections("Error in getPublications2, incorrect list of publications : "+result.publications.length);
		}
	}
}