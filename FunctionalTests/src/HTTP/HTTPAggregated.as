package HTTP
{
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.ProgressEvent;
	import flash.events.SecurityErrorEvent;
	import flash.events.TimerEvent;
	import flash.net.Socket;
	import flash.utils.Timer;

	public class HTTPAggregated extends Test
	{
		private var _host:String;
		private var _url:String;
		private var _sock:Socket = null;
		private var _message:String;
		
		private var _tabResults:Array = ["[1,2,3]","[4,5,6]","[7,8,9]"];
		private var _currentResult:int = 0;			
		
		public function HTTPAggregated(app:FunctionalTests,host:String,url:String) {
			super(app, "HTTPAggregated", "Send three HTTP request in the same packet");
			_host = host;
			_url = url;
			
			_message = "POST "+_url+" \r\nContent-Type:application/json\r\nContent-Length:7\r\n\r\n[1,2,3]"; // First message
			_message += "POST "+_url+" \r\nContent-Type:application/json\r\nContent-Length:7\r\n\r\n[4,5,6]"; // Second message
			_message += "POST "+_url+" \r\nContent-Type:application/json\r\nContent-Length:7\r\n\r\n[7,8,9]"; // Third message
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);				
			_sock = new Socket();
			_sock.addEventListener(Event.CLOSE, onEvent);
			_sock.addEventListener(Event.CONNECT, onEvent);
			_sock.addEventListener(IOErrorEvent.IO_ERROR, onEvent);
			_sock.addEventListener(SecurityErrorEvent.SECURITY_ERROR, onEvent);
			_sock.addEventListener(ProgressEvent.SOCKET_DATA, onEvent);
			
			_currentResult = 0;
			_sock.connect(_host, 80);
		}
		
		// Parse HTTP result to get only the content part 
		private function parseResult(result:String):void {
			
			var contents:Array = result.match(/(\[.*\])/g);
			for (var i:int = 0; i < contents.length; i++) {
				_app.INFO("Result : "+contents[i]);
				if (contents[i] != _tabResults[_currentResult]) { // Error
					_sock.close();
					onResult({err:"Unexpected result : "+result});
					return;
				}
				
				_currentResult++;
				if (_currentResult >= _tabResults.length) { // Finished!
					_sock.close();
					onResult({});
					return;
				}
			}
		}
		
		private function onEvent(evt:Event):void {
			
			switch(evt.type) {
				case Event.CONNECT :
					_sock.writeUTFBytes(_message);
					break;
				case Event.CLOSE :
					onResult({err:evt.toString()+" (step "+_currentResult+"/3)"});
					break;
				case ProgressEvent.SOCKET_DATA :
					_app.INFO("Received "+_sock.bytesAvailable+" bytes");
					var result:String = _sock.readUTFBytes(_sock.bytesAvailable);
					parseResult(result);
					break;
				default:
					onResult({err:evt.toString()+" (step "+_currentResult+"/3)"});
			}
		}
	}
}