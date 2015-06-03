package RTMFP
{
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.utils.Timer;
	import mx.rpc.events.FaultEvent;
	import mx.rpc.events.ResultEvent;
	import flash.net.NetConnection;
	import mx.rpc.http.HTTPService;
	
	import mx.controls.Alert;
	
	public class WriterDestruction extends Test
	{
		private var _host:String;
		private var _url:String;
		private var _connection:NetConnection = null;
	
		public function WriterDestruction(app:FunctionalTests, host:String, url:String)
		{
			super(app, "WriterDestruction", "Check if RMTFP's writer destruction is working well");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// 1) Connect with RTMFP
			_connection = new NetConnection();
			_connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection.connect("rtmfp://" + _host + _url + "WriterDestruction/");
		}
		
		// Close the connection (delayed)
		private function closeConnection(event:TimerEvent):void {
			event.target.stop();
			_connection.close();
		}
		
		private function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					// 2) Close the RTMFP connection
					var timerClose:Timer = new Timer(100);
					timerClose.addEventListener(TimerEvent.TIMER, closeConnection, false, 0, true);
					timerClose.start();
					break;
				case "NetConnection.Connect.Closed":
					_connection.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
					_connection = null;
					
					// 3) Connect with HTTP (to start the garbage collection)
					var http:HTTPService = new HTTPService();
					http.url = "http://" + _host + _url + "WriterDestruction/";
					http.method = "GET";
					http.resultFormat = "text";
					http.showBusyCursor = true;
					http.addEventListener(ResultEvent.RESULT, manageResponse);
					http.addEventListener(FaultEvent.FAULT, manageResponse);
					http.send();
					break;
				default:
					onResult({err:event.info.code});
			}
		}
		
		private function manageResponse(event:Event):void {
			
			switch(event.type) {
			case ResultEvent.RESULT: // We got a response
				var result:ResultEvent = ResultEvent(event);
				
				if (result.statusCode == 200)
					onResult( { } ); // Test Terminated!
				else
					onResult({err:"Unexpected status code : " + result.statusCode + " (expected 200)"});
				break;
			case FaultEvent.FAULT:
				onResult({err:FaultEvent(event).fault.faultString});
				break;
			}
		}

	}
}