package HTTP
{
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.SecurityErrorEvent;
	import flash.events.TimerEvent;
	import flash.net.Socket;
	import flash.system.Security;
	import flash.utils.Timer;
	
	import mx.controls.Alert;

	public class HTTPBadRequests extends Test
	{
		private var _host:String;
		private var _sock:Socket = null;
		private var _counter:int = 0;
		
		public function HTTPBadRequests(app:FunctionalTests,host:String) {
			super(app, "HTTPBadRequests", "Send 10 bad HTTP requests");
			_host = host;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_sock = new Socket();
			_sock.addEventListener(Event.CLOSE, onEvent);
			_sock.addEventListener(Event.CONNECT, onEvent);
			_sock.addEventListener(IOErrorEvent.IO_ERROR, onEvent);
			_sock.addEventListener(SecurityErrorEvent.SECURITY_ERROR, onEvent);
			
			_sock.timeout = 1000; // timeout of 1s
			_sock.connect(_host, 80);
			_counter = 0;
		}
		
		private function reconnect(evt:TimerEvent):void {
			_sock.connect(_host, 80);
		}
		
		private function onEvent(evt:Event):void {
			
			switch(evt.type) {
				case Event.CONNECT :
					_sock.writeUTFBytes("TESTBADREQUESTS");
					break;
				case Event.CLOSE :
					_counter++;
					if (_counter < 10) {
						var myTimer:Timer = new Timer(100, 1); // 0.5 second
						myTimer.addEventListener(TimerEvent.TIMER, reconnect);
						myTimer.start();
					} else
						onResult({}); // End of tests!
					break;
				default:
					onResult({err:evt.toString()});
			}
		}
	}
}