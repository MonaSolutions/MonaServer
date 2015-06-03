package HTTP
{
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.ProgressEvent;
	import flash.events.SecurityErrorEvent;
	import flash.net.Socket;
	
	public class Cookies extends Test
	{
		private var _sock:Socket;
		private var _host:String;
		private var _url:String;
		
		private var _cookies:Array = [[
			{"key":"cookie1", "value":"thisisastring", "expires":86400, "secure":false, "httponly":false, "path":"/"},
			{"key":"cookie2", "value":30, "expires":60, "secure":true, "httponly":true, "path":"/test/test"},
			]];
		private var _message:String;
		
		public function Cookies(app:FunctionalTests, host:String, url:String)
		{
			super(app, "Cookies", "Check HTTP cookies functionality");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_sock = new Socket();
			_sock.addEventListener(Event.CLOSE, onEvent);
			_sock.addEventListener(Event.CONNECT, onEvent);
			_sock.addEventListener(ProgressEvent.SOCKET_DATA, onEvent);
			_sock.addEventListener(IOErrorEvent.IO_ERROR, onEvent);
			_sock.addEventListener(SecurityErrorEvent.SECURITY_ERROR, onEvent);
			
			_sock.timeout = 1000; // timeout of 1s
			_sock.connect(_host, 80);
			
			// Prepare parameters
			var params:String = JSON.stringify(_cookies);
			_message = "POST "+_url+"cookies/ \r\nContent-Type:application/json\r\n";
			_message += "Cookie: sendCookie1=Marcel Pagnol; sendCookie2=Charles-Pierre Baudelaire; sendCookie3=one=1&two=2\r\n";
			_message += "Content-Length: "+params.length + "\r\n\r\n";
			_message += params;
		}
		
		// Parse HTTP result searching for cookies
		private function parseResult(result:String):Boolean {
			var pattern:RegExp = /HTTP\/1.1 (\d+)/g;
			var statusCode:Object = pattern.exec(result);
			if (statusCode[1] != "200") {
				onResult({err:"Error on server side (status code : "+statusCode[1]+"), check the logs."});
				return false;
			}
			
			var headers:Array = result.match(/Set-Cookie: .*/g);
			for (var i:int = 0; i < headers.length; i++) {
				var cookie:Object = _cookies[0][i];
				
				// Search for Set-Cookie: <key>=<value>
				var expectedFirst:String = "Set-Cookie: "+cookie.key+"="+cookie.value; // The expected first pair in Set-Cookie:
				if (headers[i].substr(0, expectedFirst.length) != expectedFirst) {
					_sock.close();
					onResult({err:"Expected '"+expectedFirst+"' and got '"+headers[i].substr(0, expectedFirst.length)+"'"});
					return false;
				}
				// Check HTTPOnly
				if (cookie.httponly && headers[i].indexOf("HttpOnly") < 0) {
					_sock.close();
					onResult({err:"Cookie "+cookie.key+" HttpOnly flag not found"});
					return false;
				}
				// Check Secure
				if (cookie.secure && headers[i].indexOf("Secure") < 0) {
					_sock.close();
					onResult({err:"Cookie "+cookie.key+" Secure flag not found"});
					return false;
				}
				// Check Path
				if (headers[i].indexOf("Path=" + cookie.path) < 0) {
					_sock.close();
					onResult({err:"Cookie "+cookie.key+" path is incorrect ("+cookie.path+" expected)"});
					return false;
				}
				_app.INFO(cookie.key + "=" + cookie.value + " OK.");
			}
			return true;
		}
		
		public function onEvent(event:Event):void {
			
			switch(event.type) {
			case Event.CONNECT :
				_sock.writeUTFBytes(_message);
				break;
			case ProgressEvent.SOCKET_DATA:
				var result:String = _sock.readUTFBytes(_sock.bytesAvailable);
				if (parseResult(result)) {
					_sock.close();
					onResult({});
				}
				break;
			case IOErrorEvent.IO_ERROR:
				onResult({err:IOErrorEvent(event).text});
				break;
			}
		}
	}
}