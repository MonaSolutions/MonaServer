package RTMP
{
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	
	public class RTMPTests extends Test {
	
		private var _protocol:String;
		private var _host:String;
		
		public function RTMPTests(app:FunctionalTests, host:String, protocol:String) {
			
			super(app, protocol + "Tests", "Try to connect to a wrong application");
			_protocol = protocol;
			_host = host;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Prepare POST request
			var connection:NetConnection = new NetConnection();
			connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			connection.connect(_protocol.toLocaleLowerCase() + "://" + _host + "/abcdefghijkl/");
			
		}

		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.InvalidApp":
				case "NetConnection.Connect.Failed":
					onResult({});
					break;
				default:
					onResult({err:event.info.code});
			}
			
			if(event.target is NetConnection)
				event.target.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
		}
	}
}