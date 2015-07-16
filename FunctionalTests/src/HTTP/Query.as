package HTTP
{	
	import flash.events.Event;
	
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;

	public class Query extends Test
	{
		private var _url:String;
		private var _host:String;
		private var _http:HTTPService = null;
		
		public function Query(app:FunctionalTests,host:String, url:String) {
			super(app, "Query", "Check a complex http query");
			_host = host;
			_url = url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Prepare POST request
			_http = new HTTPService();
			_http.url = "http://" + _host + _url + "onMessage?longquery://test;:/one%*$^/fin=value~";
			_http.resultFormat = "text";
			_http.method = "GET";
			_http.contentType = "application/json";
			_http.showBusyCursor = true;
			_http.addEventListener(ResultEvent.RESULT, manageResponse);
			_http.addEventListener(FaultEvent.FAULT, manageResponse);
			
			// Start deserialization
			_http.send();
		}
		
		public function manageResponse(event:Event):void {
			
			switch(event.type) {
				case ResultEvent.RESULT: // We got a response
					
					var result:String = String(ResultEvent(event).result);
					
					if (result=="[{\"longquery://test;:/one%*$^/fin\":\"value~\"}]")
						onResult({});
					else
						onResult({err:"Result ( "+result+" ) is different from message!"});
					break;
				case FaultEvent.FAULT:
					onResult({err:FaultEvent(event).fault.faultString});
					break;
			}
		}
	}
}