package
{
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.net.NetConnection;
	import flash.utils.Timer;
	
	import mx.controls.Alert;
	
	public class RTMPLoad extends Test
	{
		private var _conn:NetConnection;
		private var _host:String;
		private var _url:String;
	
		private var _countSuccess:uint = 0;
		private const NB_LOAD_TESTS:int = 1000;
	
		public function RTMPLoad(app:FunctionalTests, host:String, url:String)
		{
			super(app, "RTMPLoad", "Send 100 RTMP connections requests");
			_host=host;
			_url=url;
		}
		
		override public function run(onResult:Function):void {
			
			super.run(onResult);
			
			_countSuccess = 0;
			
			// Prepare POST request

			var connection:NetConnection = new NetConnection();
			connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			connection.connect("rtmpe://" + _host + _url);
			
		}

		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":

					if(++_countSuccess==NB_LOAD_TESTS) {
						_onResult(""); // Test Terminated!
						break;
					}
					
					var connection:NetConnection = new NetConnection();
					connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
					connection.connect("rtmpe://" + _host + _url);
					
					
					break;
				default:
					_onResult(event.info.code);
			}
			
			if(event.target is NetConnection)
				event.target.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
		}

	}
}