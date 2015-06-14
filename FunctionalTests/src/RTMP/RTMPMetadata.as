package RTMP
{
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	import flash.net.NetStream;
	import flash.events.AsyncErrorEvent;
	
	public class RTMPMetadata extends Test {
	
		private var _protocol:String;
		private var _host:String;
		private var _url:String;
		
		private var _step:int = 0; // 0 : not ready, 1 : waiting for metadata on listener 1, 2 : waiting for metadata on listener 2, 3 : waiting for metadata on listener 3
		
		private var _connection1:NetConnection;
		private var _connection2:NetConnection;
		private var _connection3:NetConnection;
		private var _out:NetStream;
		private var _in1:NetStream;
		private var _in2:NetStream;
		private var _in3:NetStream;
		
		public function RTMPMetadata(app:FunctionalTests, host:String, protocol:String, url:String) {
			
			super(app, protocol + "Metadata", "Try to get metada from multiple listeners");
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
			_step = 0;
		}
		
		// Close all connections properly
		private function terminateConnections(error:String):void {
			
			if (_out) {
				_out.removeEventListener(NetStatusEvent.NET_STATUS, onStatusOut);
				_out.close();
				_out = null;
			}
			if (_in1) {
				_in1.removeEventListener(NetStatusEvent.NET_STATUS, onStatusIn);
				_in1.removeEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusIn);
				_in1.close();
				_in1 = null;
			}
			if (_in2) {
				_in2.removeEventListener(NetStatusEvent.NET_STATUS, onStatusIn);
				_in2.removeEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusIn);
				_in2.close();
				_in2 = null;
			}
			if (_in3) {
				_in3.removeEventListener(NetStatusEvent.NET_STATUS, onStatusIn);
				_in3.removeEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusIn);
				_in3.close();
				_in3 = null;
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
		
		// Called by MonaServer
		public function onMetaData(metaData:Object):void {
			
			_app.INFO("MetaData received, step " + _step);
			if (metaData["test1"] != "value1")
				terminateConnections("Unexpected value of metaData test1 : " + metaData["test1"]);
			else if (metaData["test2"] != "value2")
				terminateConnections("Unexpected value of metaData test2 : " + metaData["test2"]);
			else {
				if (_step == 1) { // MetaData received on listener 1 => connect listener 2
					_in2 = new NetStream(_connection2);
					_in2.addEventListener(NetStatusEvent.NET_STATUS, onStatusIn);
					_in2.addEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusIn);
					_in2.client = this;
					_step = 2;
					_in2.play("test");
				} else if (_step == 2) { // MetaData received on listener 2 => connect listener 3					
					_in3 = new NetStream(_connection3);
					_in3.addEventListener(NetStatusEvent.NET_STATUS, onStatusIn);
					_in3.addEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusIn);
					_in3.client = this;
					_step = 3;
					_in3.play("test");
				} else if (_step == 3) { // MetaData received on listener 3 => Tada!
					terminateConnections("");
				}
			}			
		}
		
		public function message(dummy:String):void {
			//_app.INFO("Message received : " + dummy);
		}

		private function onStatus(event:NetStatusEvent):void {
			//_app.INFO("onStatus : " + event.info.code);
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					if (event.target==_connection1) {
						_out = new NetStream(_connection1);
						_out.addEventListener(NetStatusEvent.NET_STATUS, onStatusOut);
						_out.publish("test");
					}
					break;
				default:
					onResult({err:event.info.code});
			}
		}
		
		private function onStatusOut(event:NetStatusEvent):void {
			
			//_app.INFO("onStatusOut : " + event.info.code);
			switch(event.info.code) {
				case "NetStream.Publish.Start":
					_app.INFO("Starting listener 1...");
					// Publication create, we can create the 1st listener
					_in1 = new NetStream(_connection1);
					_in1.addEventListener(NetStatusEvent.NET_STATUS, onStatusIn);
					_in1.addEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusIn);
					_in1.client = this;
					_in1.play("test");
					break;
				default:
					onResult({err:event.info.code});
			}
		}
		
		private function onStatusIn(evt:Event):void {
			
			if (evt is AsyncErrorEvent)
				onResult( { err:AsyncErrorEvent(evt).text } ); 
			else {
				var event:NetStatusEvent = NetStatusEvent(evt);
				//_app.INFO("onStatusIn : " + event.info.code);
				switch(event.info.code) {
					case "NetStream.Play.Start":
						_out.send("message", "test"); // send a useless message to get the PublishNotify event
						break;
					case "NetStream.Play.PublishNotify":
						if (_step == 0) {
							var metaData:Object = new Object();
							metaData["test1"] = "value1";
							metaData["test2"] = "value2";
							_step=1; // wait for metadata on _in1
							_out.send("@setDataFrame", "onMetaData", metaData);
						}
					break;
				}
			}
		}
	}
}