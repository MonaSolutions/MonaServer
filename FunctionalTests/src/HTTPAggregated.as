package
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
		private var _step:int = 0;
		
		public function HTTPAggregated(app:FunctionalTests,host:String,url:String) {
			super(app, "HTTPAggregated", "Send three HTTP request in the same packet");
			_host = host;
			_url = url;
			
			_message = "POST "+_url+" \r\nContent-Type:application/json\r\nContent-Length:7\r\n\r\n[1,2,3]"; // First message
			_message += "POST "+_url+" \r\nContent-Type:application/json\r\nContent-Length:7\r\n\r\n[4,5,6]"; // Second message
			_message += "POST "+_url+" \r\nContent-Type:application/json\r\nContent-Length:7\r\n\r\n[7,8,9]"; // Third message
		}
		
		override public function run(onResult:Function):void {
			
			super.run(onResult);				
			_sock = new Socket();
			_sock.addEventListener(Event.CLOSE, onEvent);
			_sock.addEventListener(Event.CONNECT, onEvent);
			_sock.addEventListener(IOErrorEvent.IO_ERROR, onEvent);
			_sock.addEventListener(SecurityErrorEvent.SECURITY_ERROR, onEvent);
			_sock.addEventListener(ProgressEvent.SOCKET_DATA, onEvent);
			
			_step = 0;
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
					_sock.writeUTFBytes(_message);
					break;
				case Event.CLOSE :
					_onResult(evt.toString()+" (step "+_step+"/3)");
					break;
				case ProgressEvent.SOCKET_DATA :
					var result:String = _sock.readUTFBytes(_sock.bytesAvailable);
					result = parseResult(result);
					if (result=="[1,2,3]" && _step==0)
						_step=1;
					else if (result=="[4,5,6]" && _step==1)
						_step=2;
					else if (result=="[7,8,9]" && _step==2) {
						_sock.close();
						_onResult("");
					} else {
						_onResult("Unexpected result : "+result);											
					}
					break;
				default:
					_onResult(evt.toString()+" (step "+_step+"/3)");
			}
		}
	}
}