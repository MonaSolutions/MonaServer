package HTTP
{
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.ProgressEvent;
	import flash.events.SecurityErrorEvent;
	import flash.events.TimerEvent;
	import flash.net.Socket;
	import flash.utils.Timer;

	public class HTTPSplitted extends Test
	{
		private var _host:String;
		private var _url:String;
		private var _sock:Socket = null;
		
		private var _tab:Array;
		private var _indexRequest:uint;
		
		public function HTTPSplitted(app:FunctionalTests,host:String,url:String) {
			super(app, "HTTPSplitted", "Send splitted HTTP requests to Mona and check result");
			_host = host;
			_url = url;
			
			_tab = ["POST "+_url, " \r\nContent-Type:applica", "tion/json\r\nContent-Length:7\r\n", "\r\n[1,2,3]"];
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);				
			_sock = new Socket();
			_sock.addEventListener(Event.CLOSE, onEvent);
			_sock.addEventListener(Event.CONNECT, onEvent);
			_sock.addEventListener(IOErrorEvent.IO_ERROR, onEvent);
			_sock.addEventListener(SecurityErrorEvent.SECURITY_ERROR, onEvent);
			_sock.addEventListener(ProgressEvent.SOCKET_DATA, onEvent);
			
			_indexRequest = 0;
			_sock.connect(_host, 80);
		}
		
		// Parse HTTP result to get only the content part 
		private function parseResult(result:String):String {
			
			var tab:Array = result.split("\r\n\r\n");
			if (tab.length > 1)
				return tab[1];
			
			return ""
		}
		
		private function onEvent(evt:Event):void {
			
			switch(evt.type) {
				case Event.CONNECT :
					// Send first part
					_app.INFO("Sending part 1/"+_tab.length+"...");
					_sock.writeUTFBytes(_tab[_indexRequest]);
					_sock.flush();
					var timer:Timer = new Timer(2000); //2s
					timer.addEventListener(TimerEvent.TIMER, onTimer, false, 0, true);
					timer.start();
					break;
				case Event.CLOSE :
					onResult(evt.toString());
					break;
				case ProgressEvent.SOCKET_DATA :
					var result:String = _sock.readUTFBytes(_sock.bytesAvailable);
					result = parseResult(result);
					if (result=="[1,2,3]") {
						_sock.close();
						onResult({});
					} else
						onResult({err:"Unexpected result : "+result});
					break;
				default:
					_sock.close();
					onResult({err:evt.toString()});
			}
		}
		
		private function onTimer(evt:TimerEvent):void {
			
			// Send next part of HTTP request
			_sock.writeUTFBytes(_tab[++_indexRequest]);
			_app.INFO("Sending part "+(_indexRequest+1)+"/"+_tab.length+"...");
			_sock.flush();
			
			if (_indexRequest >= _tab.length-1)
				evt.target.stop();
		}
	}
}