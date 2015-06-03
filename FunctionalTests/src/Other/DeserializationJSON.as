package Other
{
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	
	import mx.controls.Alert;
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;
	
	public class DeserializationJSON extends Test
	{
		private var _url:String;
		private var _host:String;
		
		private var _http:HTTPService = null;
		
		private var _tabJSON:Array = [
			"[{\"x\":10,\"y\":10,\"width\":100,\"height\":100}]",
			"[[10,10,100,100]]",
			"[[{\"x\":10,\"y\":10},100,100]]",
			"[[100,100,{\"x\":10,\"y\":10}]]",
			"[[{\"x\":[100,100],\"y\":[10,10]}]]",
			"[[{\"x\":[{\"y\":[10,10]},15]},25]]"
		];
		private var _currentIndex:int;
		
		public function DeserializationJSON(app:FunctionalTests,host:String, url:String)
		{
			super(app, "DeserializationJSON", "Send JSON data and check received result");
			
			_host = host;
			_url = url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Prepare POST request
			_http = new HTTPService();
			_http.url = "http://" + _host + _url;
			_http.resultFormat = "text";
			_http.method = "POST";
			_http.contentType = "application/json";
			_http.showBusyCursor = true;
			_http.addEventListener(ResultEvent.RESULT, manageResponse);
			_http.addEventListener(FaultEvent.FAULT, manageResponse);
			
			startDeserialization();
		}
		
		public function startDeserialization():void { 
			
			_currentIndex = 0;
			var json:String = _tabJSON[_currentIndex];
			_http.send(json);
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
				case ResultEvent.RESULT: // We got a response
					
					var result:String = String(ResultEvent(event).result);
					var original:String = _tabJSON[_currentIndex];
					
					// Response valid (we use JSON parser to compare the two objects)
					if (Equals(JSON.parse(result), JSON.parse(original)) && Equals(JSON.parse(original), JSON.parse(result))) {
						
						_currentIndex += 1;
						if (_currentIndex < _tabJSON.length) {
							var json:String = _tabJSON[_currentIndex];
							_http.send(json);
						} else
							onResult({}); // Test Terminated!
					} else
						onResult({err:"Expected '"+original+"' and got '"+result+"'"});
					break;
				case FaultEvent.FAULT:
					onResult({err:FaultEvent(event).fault.faultString});
					break;
			}
		}
	}
}