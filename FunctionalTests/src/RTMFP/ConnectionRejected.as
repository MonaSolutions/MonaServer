package RTMFP
{
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.net.NetConnection;
	import flash.utils.Timer;
	
	import mx.controls.Alert;
	
	public class ConnectionRejected extends Test
	{
		private var _host:String;
		private var _url:String;
	
		private var _countSuccess:int = 0;
		private const NB_LOAD_TESTS:int = 100;
	
		public function ConnectionRejected(app:FunctionalTests, host:String, url:String)
		{
			super(app, "ConnectionRejected", "Send 100 RTMFPLoad connections requests and expect 100 rejections");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_countSuccess = 0;
			
			var connection:NetConnection = new NetConnection();
			connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus, false, 0, true);
			connection.connect("rtmfp://" + _host + _url + "ConnectionRejected/");
		}
		
		public function onStatus(event:NetStatusEvent):void {
			
			trace(event.info.code);
			switch(event.info.code) {
				case "NetConnection.Connect.Closed":
					break;
				case "NetConnection.Connect.Rejected":
					if(++_countSuccess==NB_LOAD_TESTS) {	
						
						onResult({}); // Test Terminated!
						break;
					} else {
						var connection:NetConnection = new NetConnection();
						connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus, false, 0, true);
						connection.connect("rtmfp://" + _host + _url + "ConnectionRejected/");						
					}
					break;
				default:
					onResult({err:event.info.code});
			}
		}

	}
}