package
{
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.net.NetConnection;
	import flash.utils.Timer;
	
	import mx.controls.Alert;
	
	public class RTMFPLoad extends Test
	{
		private var _host:String;
		private var _url:String;
	
		private var _countSuccess:int = 0;
		private const NB_LOAD_TESTS:int = 100;
	
		public function RTMFPLoad(app:FunctionalTests, host:String, url:String)
		{
			super(app, "RTMFPLoad", "Send 100 RTMFPLoad connections requests");
			_host=host;
			_url=url;
		}
		
		override public function run(onResult:Function):void {
			
			super.run(onResult);
			
			// Prepare POST request
			
			_countSuccess = 0;
			
			for(var i:uint;i<NB_LOAD_TESTS;++i) {
				var connection:NetConnection = new NetConnection();
				connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
				connection.connect("rtmfp://" + _host + _url);
			}

		}

		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					if(++_countSuccess==NB_LOAD_TESTS) {
						_onResult(""); // Test Terminated!
						break;
					}
					break;
				default:
					_onResult(event.info.code);
			}
			
			if(event.target is NetConnection)
				event.target.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
		}

	}
}