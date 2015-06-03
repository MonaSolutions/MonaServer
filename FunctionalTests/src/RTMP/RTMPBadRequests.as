package RTMP
{
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.ProgressEvent;
	import flash.events.SecurityErrorEvent;
	import flash.events.TimerEvent;
	import flash.net.Socket;
	import flash.system.Security;
	import flash.system.System;
	import flash.utils.Timer;
	import flash.utils.setTimeout;
	
	import mx.controls.Alert;
	import mx.rpc.CallResponder;

	public class RTMPBadRequests extends Test
	{
		private var _host:String;
		private var _sock:Socket = null;
		private var _counter:int = 0;
		private var _message:String;
		
		public function RTMPBadRequests(app:FunctionalTests,host:String) {
			super(app, "RTMPBadRequests", "Send 10 bad RTMP requests");
			_host = host;
			
			// Prepare message (size must be > to 1537 otherwise Mona wait for next characters!
			for (var i:int = 0; i < 2000; i++) {
				_message += "a";
			}
		}
		
		override public function run(onFinished:Function):void {
			super.run(onFinished);
			
			// First connect
			_sock = new Socket();
			_sock.addEventListener(Event.CLOSE, onEvent);
			_sock.addEventListener(Event.CONNECT, onEvent);
			_sock.addEventListener(IOErrorEvent.IO_ERROR, onEvent);
			_sock.addEventListener(SecurityErrorEvent.SECURITY_ERROR, onEvent);
			
			_sock.timeout = 1000; // timeout of 1s
			_sock.connect(_host, 1935);
			_counter = 0;
		}
		
		private function reconnect(evt:TimerEvent):void {
			_sock.connect(_host, 1935);
		}
		
		private function onEvent(evt:Event):void {
			
			switch(evt.type) {
				case Event.CONNECT :
					_sock.writeUTFBytes(_message);
					break;
				case Event.CLOSE :
					_counter++;
					if (_counter < 10) {
						var myTimer:Timer = new Timer(500, 1); // 0.5 second
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