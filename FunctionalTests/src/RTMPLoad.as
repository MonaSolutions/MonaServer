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
		private var _timer:Timer;
		
		private var _currentTest:int = 0;
		private const NB_LOAD_TESTS:int = 100;
		private var _ncs:Array = new Array();
		
		public function RTMPLoad(app:FunctionalTests, host:String, url:String)
		{
			super(app, "RTMPLoad", "Send 100 RTMP connections requests");
			_host=host;
			_url=url;
		}
		
		override public function run(onResult:Function):void {
			
			super.run(onResult);
			
			// Prepare POST request
			_conn = new NetConnection();
			_conn.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_conn.connect("rtmp://" + _host + _url);
			
			_timer = new Timer(1);
			_timer.addEventListener(TimerEvent.TIMER, onReady);
			
			_currentTest = 0;
		}
		
		private function closeConnections():void {
			
			_timer.stop();
			for(var conn:Object in _ncs) {
				var connection:NetConnection = conn as NetConnection;
				if (connection && connection.connected)
					connection.close();
			}
		}
		
		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
				
					if (_currentTest < NB_LOAD_TESTS) {
						_currentTest += 1;
						
						// Add connection and go to next one
						_ncs.push(_conn);
						_timer.start();
					} else {
						
						// Close all connections
						closeConnections();
						_onResult(""); // Test Terminated!
					}
					break;
				case "NetConnection.Connect.Closed":
					break;
				default:
					closeConnections();
					_onResult(event.info.code);
			}
		}
		
		public function onReady(event:TimerEvent):void {
		
			_timer.stop();
			_conn = new NetConnection();
			_conn.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_conn.connect("rtmp://" + _host + _url);
		}
	}
}