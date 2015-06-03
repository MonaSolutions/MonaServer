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
		
		private var _listenerCounter:int = 0;
		private var _connectionCounter:int = 0;
	
		public function Publications(app:FunctionalTests, host:String, url:String) {
			super(app, "Publications", "Test publications and listeners attributes");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Step 1 : Connections
			_connection1 = new NetConnection();
			_connection1.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection1.connect("rtmfp://" + _host + _url + "Publications");
			
			_connection2 = new NetConnection();
			_connection2.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection2.connect("rtmfp://" + _host + _url + "Publications");
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
			
			if (error.length > 0)
				onResult( { err: error } );
			else
				onResult( { } );
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
						// Step 4 : check variables
						if (_listenerCounter == 2) {
						 	_connection1.call("getPublications", new Responder(onPublications,terminateConnections));
						}
						break;
				default:
					break;
			}
		}
		
		// Callback functions
		private function onPublications(result:Object):void {
			
			if (result.publications.length == 1 && result.publications[0] == "publication") {
				
				if (result.listeners.length == 2) {
					if ((result.listeners[0] == _connection1.nearID && result.listeners[1] == _connection2.nearID) ||
						(result.listeners[0] == _connection2.nearID && result.listeners[1] == _connection1.nearID))
						terminateConnections("");
					else
						terminateConnections("Error in getPublications, incorrect list of listeners");
					
				} else 
					terminateConnections("Error in getPublications, incorrect number of listeners : "+result.listeners.length);
			}
			else
				terminateConnections("Error in getPublications, incorrect list of publications : "+result.publications.length);
		}
	}
}