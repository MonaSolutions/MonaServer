package HTTP
{
	import flash.events.Event;
	
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;
	
	public class HTTPReconnect extends Test
	{
		private var _parameters:Object = new Object();
		private var _http:HTTPService;
		private var _host:String;
		private var _url:String;
		
		private var _appUrl:String;
		private var _subappUrl:String;
		
		private var _currentTest:int = 0;
		private const NB_LOAD_TESTS:int = 100; 
		
		public function HTTPReconnect(app:FunctionalTests, host:String, url:String)
		{
			super(app, "HTTPReconnect", "Connect and reconnect in app and child app");
			_host=host;
			_url=url;
			_appUrl = "http://" + _host + _url; 
			_subappUrl = "http://" + _host + _url + "subapp/";
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Prepare POST request
			_http = new HTTPService();
			_http.url = _appUrl;
			_http.method = "POST";
			_http.resultFormat = "flashvars";
			_http.showBusyCursor = true;
			_http.addEventListener(ResultEvent.RESULT, manageResponse);
			_http.addEventListener(FaultEvent.FAULT, manageResponse);
			
			// Prepare parameters
			_parameters["var1"] = "value1";
			
			_currentTest = 0;
			_http.send(_parameters);
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
			case ResultEvent.RESULT: // We got a response
				var result:Object = ResultEvent(event).result;
				if (result["var1"] == "value1") { // Valid Response 
					
					if (_currentTest < NB_LOAD_TESTS) {
						_currentTest += 1;
						
						// Disconnection and reconnection
						_http.disconnect();
						_http.url = (_http.url ==  _appUrl) ? _subappUrl : _appUrl;
						_http.send(_parameters);
					} else
						onResult({}); // Test Terminated!
				} else
					onResult({err:"Error in result ("+result.toString()+")"});
				break;
			case FaultEvent.FAULT:
				onResult({err:FaultEvent(event).fault.faultString});
				break;
			}
		}
	}
}