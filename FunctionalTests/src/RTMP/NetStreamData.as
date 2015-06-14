package RTMP
{
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	import flash.net.NetStream;
	import flash.events.AsyncErrorEvent;
	
	public class NetStreamData extends Test {
	
		private var _protocol:String;
		private var _host:String;
		private var _url:String;
		
		private var _inStartCounter:int = 0; // Counter of started netstreams
		private var _inUnpublishCounter:int = 0; // Counter of unpublish events
		private var _inDataCounter:int = 0; // Counter of messages received
		private var _nbConnections:int = 0;	// Counter of ready connections 
		
		private var _connection1:NetConnection;
		private var _connection2:NetConnection;
		private var _connection3:NetConnection;
		private var _out:NetStream;
		private var _nsArray:Array = new Array();
		
		public function NetStreamData(app:FunctionalTests, host:String, protocol:String, url:String) {
			
			super(app, protocol + "NetStreamData", "Send data to listeners and check reception");
			_protocol = protocol;
			_host = host;
			_url = url;
		}
		
		override public function run(onFinished:Function):void {
			super.run(onFinished);
			
			// Create 3 connections
			_connection1 = new NetConnection();
			_connection1.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection1.connect(_protocol.toLocaleLowerCase() + "://" + _host + _url);
			
			_connection2 = new NetConnection();
			_connection2.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection2.connect(_protocol.toLocaleLowerCase() + "://" + _host + _url);
			
			_connection3 = new NetConnection();
			_connection3.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection3.connect(_protocol.toLocaleLowerCase() + "://" + _host + _url);
			
			_inStartCounter = 0;
			_inDataCounter = 0;
			_inUnpublishCounter = 0;
			_nbConnections = 0;
		}
		
		// Close all connections properly
		private function terminateConnections(error:String):void {
			
			_nsArray.splice(0,3);
			if (_out) {
				_out.removeEventListener(NetStatusEvent.NET_STATUS, onStatusOut);
				_out.close();
				_out = null;
			}
			_connection1.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection1.close();
			_connection1 = null;
			_connection2.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection2.close();
			_connection2 = null;
			_connection3.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection3.close();
			_connection3 = null;
			
			if (error.length > 0)
				onResult( { err: error } );
			else
				onResult( { } );
		}
		
		public function message(dummy:String):void {
			//_app.INFO("Message received : " + dummy);
			_inDataCounter++;
			if (_inDataCounter == 3) { // Close outstream
				_out.removeEventListener(NetStatusEvent.NET_STATUS, onStatusOut);
				_out.close();
				_out = null;
			}
		}

		private function onStatus(event:NetStatusEvent):void {
			//_app.INFO("onStatus : " + event.info.code);
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					
					// Create the publisher
					_nbConnections++;
					if (_nbConnections == 3) {
						_app.INFO("Starting publisher...");
						
						_out = new NetStream(_connection1);
						_out.addEventListener(NetStatusEvent.NET_STATUS, onStatusOut);
						_out.publish("publication");
					}
					break;
				default:
					terminateConnections(event.info.code);
			}
		}
		
		private function onStatusOut(event:NetStatusEvent):void {
			
			//_app.INFO("onStatusOut : " + event.info.code);
			switch(event.info.code) {
				case "NetStream.Publish.Start":
					_app.INFO("Starting listeners...");
					
					var ns:NetStream = new NetStream(_connection1);
					_nsArray.push(ns);
					ns = new NetStream(_connection2);
					_nsArray.push(ns);
					ns = new NetStream(_connection3);
					_nsArray.push(ns);
					
					for each (var ns2:NetStream in _nsArray) {
						ns2.addEventListener(NetStatusEvent.NET_STATUS, onStatusIn);
						ns2.addEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusIn);
						ns2.addEventListener(IOErrorEvent.IO_ERROR, onStatusIn);
						ns2.client = this;
						ns2.play("publication");
					}
					break;
				case "NetStream.Unpublish.Success":
					break;
				default:
					terminateConnections(event.info.code);
			}
		}
		
		private function onStatusIn(evt:Event):void {
			
			if (evt is IOErrorEvent)
				terminateConnections("NetStream error : " + IOErrorEvent(evt).text);
			else if (evt is AsyncErrorEvent)
				terminateConnections("NetStream error : " + AsyncErrorEvent(evt).text);
			else {
				var event:NetStatusEvent = NetStatusEvent(evt);
				//_app.INFO("onStatusIn : " + event.info.code);
				switch(event.info.code) {
					case "NetStream.Play.Start":
						_inStartCounter++;
						if (_inStartCounter==3)
							_out.send("message", "test");
						break;
					case "NetStream.Play.UnpublishNotify":
						_inUnpublishCounter++;
						if (_inStartCounter==3)
							terminateConnections("");
					break;
				}
			}
		}
	}
}