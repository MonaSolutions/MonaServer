package
{
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	
	import mx.controls.Alert;
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;
	
	public class DeserializationXML extends Test
	{
		private var _url:String;
		private var _host:String;
		
		private var _http:HTTPService = null;
		
		private var _tabXML:Array = [
			{"msg":"<__array><__array><x/></__array></__array>", "expected":"<__array><__array><x/></__array></__array>"},
			{"msg":'<__array><__array><x x1="1" x2="2" x3="3"/></__array></__array>', "expected":"<__array><__array><x><x2>2</x2><x1>1</x1><x3>3</x3></x></__array></__array>"}, // Warn : order can differ
			{"msg":'<__array><__array><x x1="1" x2="2" x3="3"><y>10</y></x></__array></__array>', "expected":"<__array><__array><x><x2>2</x2><x1>1</x1><y>10</y><x3>3</x3></x></__array></__array>"},  // Warn : order can differ
			{"msg":"<__array><__array><y>10</y><x>10</x><height>100</height><width>100</width></__array></__array>","expected":"<__array><__array><y>10</y><x>10</x><height>100</height><width>100</width></__array></__array>"},
			{"msg":"<__array><__array><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></__array></__array>","expected":"<__array><__array><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></__array></__array>"},
			{"msg":"<__array><__array><__noname>10</__noname><__noname>10</__noname><__noname>100</__noname><__noname>100</__noname></__array></__array>","expected":"<__array><__array><__noname>10</__noname><__noname>10</__noname><__noname>100</__noname><__noname>100</__noname></__array></__array>"},
			{"msg":"<__array><__array><a>10</a><a>10</a><a>100</a><a>100</a></__array></__array>", "expected":"<__array><__array><a>10</a><a>10</a><a>100</a><a>100</a></__array></__array>"},
			{"msg":"<__array><__array><x/><x>10</x></__array></__array>", "expected":"<__array><__array><x/><x>10</x></__array></__array>"},
			{"msg":"<__array><__array><a>100</a><a>100</a><a><y>10</y></a><a><x>10</x></a></__array></__array>", "expected":"<__array><__array><a>100</a><a>100</a><a><y>10</y></a><a><x>10</x></a></__array></__array>"},
			{"msg":"<__array><__array><a><y>10</y></a><a><x>10</x></a><a>100</a><a>100</a></__array></__array>", "expected":"<__array><__array><a><y>10</y></a><a><x>10</x></a><a>100</a><a>100</a></__array></__array>"},
			{"msg":"<__array><__array><a><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></a></__array></__array>", "expected":"<__array><__array><a><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></a></__array></__array>"},
			{"msg":"<__array><__array><a><y>10</y><x>10</x></a><a>100</a><a>100</a></__array></__array>", "expected":"<__array><__array><a><y>10</y><x>10</x></a><a>100</a><a>100</a></__array></__array>"},
			{"msg":"<__array><__array><a><y>20</y><x>10</x>30</a></__array></__array>", "expected":"<__array><__array><a><y>20</y><x>10</x><__noname>30</__noname></a></__array></__array>"},
			{"msg":"<__array><__array><a><__array><__noname>100</__noname><__noname>100</__noname></__array></a><a>10</a><a>10</a></__array></__array>", "expected":"<__array><__array><a><__array><__noname>100</__noname><__noname>100</__noname></__array></a><a>10</a><a>10</a></__array></__array>"}
		];
		private var _currentIndex:int;
		
		public function DeserializationXML(app:FunctionalTests,host:String, url:String)
		{
			super(app, "DeserializationXML", "send XML data and check received result");
			
			_host = host;
			_url = url;
		}
		
		override public function run(onResult:Function):void {
			
			super.run(onResult);
			
			// Prepare POST request
			_http = new HTTPService();
			_http.url = _url;
			_http.resultFormat = "text";
			_http.method = "POST";
			_http.contentType = "text/xml";
			_http.showBusyCursor = true;
			_http.addEventListener(ResultEvent.RESULT, manageResponse);
			_http.addEventListener(FaultEvent.FAULT, manageResponse);
			
			startDeserialization();
		}
		
		public function startDeserialization():void { 
			
			_currentIndex = 0;
			var msg:String = _tabXML[_currentIndex].msg;
			_http.send(msg);
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
				case ResultEvent.RESULT: // We got a response
					
					var result:String = String(ResultEvent(event).result);					
					var expected:String = _tabXML[_currentIndex].expected;
					
					// Response valid (we use JSON parser to compare the two objects)
					if (result == expected) {
						
						_currentIndex += 1;
						if (_currentIndex < _tabXML.length) {
							var msg:String = _tabXML[_currentIndex].msg;
							_http.send(msg);
						} else
							_onResult(""); // Test Terminated!
					} else
						_onResult("Expected '"+expected+"' and got '"+result+"'");
					break;
				case FaultEvent.FAULT:
					_onResult(FaultEvent(event).fault.faultString);
					break;
			}
		}
	}
}
