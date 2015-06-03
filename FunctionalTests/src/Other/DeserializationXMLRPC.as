package Other
{
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	import mx.controls.Alert;
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;
	
	public class DeserializationXMLRPC extends Test
	{
		private var _url:String;
		private var _host:String;
		private var _http:HTTPService = null;
		
		private var _tabXML:Array = [
			{"msg":["<boolean>1</boolean>"]},
			{"msg":["<double>-12.53</double>"]},
			{"msg":["<i4>123456789</i4>"],
				"expected":["<int>123456789</int>"]},
			{"msg":["<base64>eW91IGNhbid0IHJlYWQgdGhpcyE=</base64>"]},
			{"msg":["<string>Hello world!</string>"]},
			{"msg":["<string>&~#{[||;`\^@]}=)àç_è-(\'\"é&^$*ù!:;,</string>"]},
			{"msg":["<dateTime.iso8601>2014-04-22T06:00:00.000+02:00</dateTime.iso8601>"]},
			{"msg":["<array size=\"3\"><data><value><int>1</int></value><value><int>2</int></value><value><int>3</int></value></data></array>"]},
			{"msg":["<int>1</int>","<int>2</int>","<int>3</int>"]},
			{"msg":["<struct><member><name>x1</name><value><int>1</int></value></member><member><name>x2</name><value><int>2</int></value></member></struct>"],
				"expected":["<struct><member><name>x2</name><value><int>2</int></value></member><member><name>x1</name><value><int>1</int></value></member></struct>"]}, // here order change in deserialization
		];
		private var _currentIndex:int;
		
		public function DeserializationXMLRPC(app:FunctionalTests,host:String, url:String)
		{
			super(app, "DeserializationXMLRPC", "Send XMLPC request and check received result");
			
			_host = host;
			_url = url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Prepare POST request
			_http = new HTTPService();
			_http.url =  "http://" + _host + _url;
			_http.resultFormat = "text";
			_http.method = "POST";
			_http.contentType = "text/xml";
			_http.showBusyCursor = true;
			_http.addEventListener(ResultEvent.RESULT, manageResponse);
			_http.addEventListener(FaultEvent.FAULT, manageResponse);
			
			// Start deserialization
			_currentIndex = 0;
			_http.send(formatMessage(_tabXML[_currentIndex]));
		}
		
		// Generate XMLRPC message to be send to Mona
		private function formatMessage(obj:Object):String {
			var values:Array = obj.msg;
			var message:String = "<?xml version=\"1.0\"?><methodCall><methodName>onMessage</methodName><params>";
			for each (var value:String in values) {
				message += "<param><value>";
				message += value;
				message += "</value></param>";
			}
			message += "</params></methodCall>";
			
			return message;
		}
		
		// Generate response expected
		private function formatExpected(obj:Object):String {
			var values:Array = (obj.expected)? obj.expected : obj.msg;
			var expected:String = "<?xml version=\"1.0\"?><methodResponse><params>";
			for each (var value:String in values) {
				expected += "<param><value>";
				expected += value;
				expected += "</value></param>";
			}
			expected += "</params></methodResponse>";
			
			return expected;
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
				case ResultEvent.RESULT: // We got a response
					
					var result:String = String(ResultEvent(event).result);					
					var expected:String = formatExpected(_tabXML[_currentIndex]);
					
					// Response valid (we use JSON parser to compare the two objects)
					if (result == expected) {
						
						_currentIndex += 1;
						if (_currentIndex < _tabXML.length)
							_http.send(formatMessage(_tabXML[_currentIndex]));
						else
							onResult({}); // Test Terminated!
					} else
						onResult({err:"Result unexpected for index "+(_currentIndex+1)});
					break;
				case FaultEvent.FAULT:
					onResult({err:FaultEvent(event).fault.faultString});
					break;
			}
		}
	}
}
