package HTTP
{
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.SecurityErrorEvent;
	import flash.events.TimerEvent;
	import flash.net.Socket;
	import flash.system.Security;
	import flash.utils.ByteArray;
	import flash.utils.Timer;
	
	import mx.controls.Alert;

	public class HTTPBadRequests extends Test
	{
		private var _host:String;
		private var _sock:Socket = null;
		private var _counter:int = 0;
		
		private var _step:int = 0;		// 2 Steps : send "TESTBDADREQUESTS", then send "\x20\x0A\x00\xFF\x0A"
		
		private var _bytes:ByteArray = null;
		
		public function HTTPBadRequests(app:FunctionalTests,host:String) {
			super(app, "HTTPBadRequests", "Send 20 bad HTTP requests");
			_host = host;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_sock = new Socket();
			_sock.addEventListener(Event.CLOSE, onEvent);
			_sock.addEventListener(Event.CONNECT, onEvent);
			_sock.addEventListener(IOErrorEvent.IO_ERROR, onEvent);
			_sock.addEventListener(SecurityErrorEvent.SECURITY_ERROR, onEvent);
			
			// ByteArray construction
			_bytes = new ByteArray();
			var bytes:Array = [int("0x20"), int("0x0A"), 0, int("0xFF"), int("0x0A")];
			for (var i:int = 0; i < bytes.length; i++) {
				_bytes.writeByte(bytes[i]);
			}
			
			_sock.timeout = 1000; // timeout of 1s
			_sock.connect(_host, 80);
			_counter = 0;
			_step = 0;
		}
		
		private function reconnect(evt:TimerEvent):void {
			_sock.connect(_host, 80);
		}
		
		private function onEvent(evt:Event):void {
			
			switch(evt.type) {
				case Event.CONNECT :
					if (_step==0)
						_sock.writeUTFBytes("TESTBADREQUESTS");
					else
						_sock.writeBytes(_bytes);
					break;
				case Event.CLOSE :
					_counter++;
					if (_counter >= 10) {
						_counter = 0;
						if (_step==1) {
							onResult( { } ); // End of tests!
							return;
						}
						_step++;
					}
					var myTimer:Timer = new Timer(100, 1); // 0.5 second
					myTimer.addEventListener(TimerEvent.TIMER, reconnect);
					myTimer.start();
					break;
				default:
					onResult({err:evt.toString()});
			}
		}
	}
}