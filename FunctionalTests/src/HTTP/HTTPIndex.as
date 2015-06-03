package HTTP
{
	import flash.events.Event;
	
	import mx.controls.Alert;
	import mx.controls.TextArea;
	import mx.rpc.AsyncToken;
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;
	
	public class HTTPIndex extends Test
	{
		private var _parameters:Object = new Object();
		private var _http:HTTPService;
		private var _host:String;
		private var _url:String;
		
		private var _currentTest:int = 0;
		
		public function HTTPIndex(app:FunctionalTests, host:String, url:String)
		{
			super(app, "HTTPIndex", "Test HTTP index returned attribute of client:onConnection");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Prepare POST request
			_http = new HTTPService();
			_http.url = "http://" + _host + _url + "HTTPIndex/";
			_http.method = "GET";
			_http.resultFormat = "text";
			_http.showBusyCursor = true;
			_http.addEventListener(ResultEvent.RESULT, manageResponse);
			_http.addEventListener(FaultEvent.FAULT, manageResponse);
			
			_currentTest = 0;
			_http.send();
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
				case ResultEvent.RESULT: // We got a response
					var status:int = ResultEvent(event).statusCode;
					_app.INFO("Status : " + status);
					switch(_currentTest) {
						case 0: // Index default (true)
							if (status != 200) {
								onResult( { err:"Step 0 - Unexpected HTTP status code : " + status + " (expected 200)" } );
								return;
							}
							_http.url = "http://" + _host + _url + "HTTPIndex/IndexFalse/";
							break;
						case 1: // Index = false (generate a FaultEvent)
							onResult( { err:"Step 1 - Unexpected HTTP response : " + status + " (error 403 expected)" } );
							break;
						case 2: // Index = true
							if (status != 200) {
								onResult( { err:"Step 2 - Unexpected HTTP status code : " + status + " (expected 200)" } );
								return;
							}
							_http.url = "http://" + _host + _url + "HTTPIndex/IndexFile/";
							break;
						case 3: // Index = file.txt
							if (status != 200)
								onResult( { err:"Step 3 - Unexpected HTTP status code : " + status + " (expected 200)" } );
							else if (ResultEvent(event).result != "one line file")
								onResult( { err:"Step 3 - Unexpected HTTP response : " + ResultEvent(event).result + " (expected 'one line file')" } );
							else
								onResult( { } ); // Test OK!
							return;
					}
					break;
				case FaultEvent.FAULT:
					var statusErr:int = FaultEvent(event).statusCode;
					_app.INFO("Error status : " + statusErr);
					if (_currentTest == 1 && statusErr == 403)
						_http.url = "http://" + _host + _url + "HTTPIndex/IndexTrue/";	
					else {
						onResult( { err:"Step "+_currentTest+" - "+event.toString() } );
						return;
					}
					break;
			}
			
			// Next test
			_currentTest++;
			_http.send();
		}
	}
}