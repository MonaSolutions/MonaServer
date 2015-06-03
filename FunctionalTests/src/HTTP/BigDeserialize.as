package HTTP
{	
	import flash.events.Event;
	
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;

	public class BigDeserialize extends Test
	{
		private var _url:String;
		private var _host:String;
		private var _http:HTTPService = null;
		private var _message:String;
		
		public function BigDeserialize(app:FunctionalTests,host:String, url:String) {
			super(app, "BigDeserialize", "Send big JSON request to be deserialized by Mona");
			_host = host;
			_url = url;
			
			// Prepare a big JSON Message
			_message = "[";
			for (var i:int = 0; i < 15000; i++) {
				_message += i + ",";
			}
			_message += i + "]";
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
			
			// Start deserialization
			_http.send(_message);
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
				case ResultEvent.RESULT: // We got a response
					
					var result:String = String(ResultEvent(event).result);					
					
					if (result==_message)
						onResult({});
					else
						onResult({err:"Result is different from message!"});
					break;
				case FaultEvent.FAULT:
					onResult({err:FaultEvent(event).fault.faultString});
					break;
			}
		}
	}
}