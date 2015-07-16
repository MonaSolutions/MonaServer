package RTMFP
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
		private var _conns:Array = new Array();
	
		public function RTMFPLoad(app:FunctionalTests, host:String, url:String)
		{
			super(app, "RTMFPLoad", "Send 100 RTMFPLoad connections requests");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_countSuccess = 0;
			
			for(var i:uint;i<NB_LOAD_TESTS;++i) {
				var connection:NetConnection = new NetConnection();
				_conns.push(connection);
				connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
				connection.connect("rtmfp://" + _host + _url);
			}

		}

		// Close connection (with RTMFP connections are still alive since closure, removeEventListener is not sufficient)
		private function closeConnections(event:TimerEvent):void {
			event.target.stop();
			for each (var conn:NetConnection in _conns) {
				conn.close();
			}
		}
		
		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					if(++_countSuccess==NB_LOAD_TESTS) {
						
						// Delayed close to close also the last connection achieved
						var timerClose:Timer = new Timer(100);
						timerClose.addEventListener(TimerEvent.TIMER, closeConnections, false, 0, true);
						timerClose.start();
						
						onResult({}); // Test Terminated!
						break;
					}
					break;
				case "NetConnection.Connect.Closed":
					break;
				default:
					onResult({err:event.info.code});
			}
		}

	}
}