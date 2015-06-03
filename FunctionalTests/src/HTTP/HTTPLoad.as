package HTTP
{
	import flash.events.Event;
	
	import mx.controls.Alert;
	import mx.controls.TextArea;
	import mx.rpc.AsyncToken;
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;
	
	public class HTTPLoad extends Test
	{
		private var _parameters:Object = new Object();
		private var _http:HTTPService;
		private var _host:String;
		private var _url:String;
		
		private var _currentTest:int = 0;
		private const NB_LOAD_TESTS:int = 100; 
		
		public function HTTPLoad(app:FunctionalTests, host:String, url:String)
		{
			super(app, "HTTPLoad", "Send 100 POST requests with 2 parameters");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Prepare POST request
			_http = new HTTPService();
			_http.url = "http://" + _host + _url;
			_http.method = "POST";
			_http.resultFormat = "flashvars";
			_http.showBusyCursor = true;
			_http.addEventListener(ResultEvent.RESULT, manageResponse);
			_http.addEventListener(FaultEvent.FAULT, manageResponse);
			
			// Prepare parameters
			_parameters["var1"] = "value1";
			_parameters["var2"] = "value2";
			
			_currentTest = 0;
			_http.send(_parameters);
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
			case ResultEvent.RESULT: // We got a response
				var result:Object = ResultEvent(event).result;
				if (result["var1"] == "value1" || result["var2"] == "value2") { // Response valid
					
					if (_currentTest < NB_LOAD_TESTS) {
						_currentTest += 1;
						_http.send(_parameters); // Next request
					} else
						onResult({}); // Test Terminated!
				} else
					onResult({err:"Error in result ("+result.toString()+")"});
				break;
			case FaultEvent.FAULT:
				onResult({err:event.toString()});
				break;
			}
		}
	}
}