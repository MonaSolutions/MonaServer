package RTMP
{
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.net.NetConnection;
	import flash.utils.Timer;
	
	public class RTMPLoad extends Test
	{
		private var _fullUrl:String;
	
		private var _countSuccess:uint = 0;
		private const NB_LOAD_TESTS:int = 100;
	
		public function RTMPLoad(app:FunctionalTests, host:String, url:String, protocol:String)
		{
			super(app, protocol + "Load", "Send 100 " + protocol + " connections requests");
			_fullUrl = protocol.toLocaleLowerCase() + "://" + host + url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_countSuccess = 0;
			
			// Prepare POST request
			var connection:NetConnection = new NetConnection();
			connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			connection.connect(_fullUrl)
			
		}

		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":

					if(++_countSuccess==NB_LOAD_TESTS) {
						onResult({}); // Test Terminated!
						break;
					}
					
					var connection:NetConnection = new NetConnection();
					connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
					connection.connect(_fullUrl);
					
					break;
				default:
					onResult({err:event.info.code});
			}
			
			if(event.target is NetConnection)
				event.target.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
		}
	}
}