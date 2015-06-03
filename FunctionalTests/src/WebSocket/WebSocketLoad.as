/************************************************************************
 * This test use the AS3WebSocket library available here :
 * https://github.com/theturtle32/AS3WebSocket.git
 * 
 ***********************************************************************/
package WebSocket
{
	import com.worlize.websocket.WebSocket;
	import com.worlize.websocket.WebSocketError;
	import com.worlize.websocket.WebSocketErrorEvent;
	import com.worlize.websocket.WebSocketEvent;
	
	public class WebSocketLoad extends Test
	{
		private var _parameters:String = '[[{"var1":"value1","var2":"value2"}]]';
		private var _websocket:WebSocket;
		private var _host:String;
		private var _url:String;
		
		private var _currentTest:int = 0;
		private const NB_LOAD_TESTS:int = 100;
		
		public function WebSocketLoad(app:FunctionalTests, host:String, url:String)
		{
			super(app, "WebSocketLoad", "Send 100 requests with 2 parameters");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_currentTest = 0;
			_websocket = new WebSocket("ws://" + _host + _url, "*");
			_websocket.addEventListener(WebSocketEvent.CLOSED, onStatus);
			_websocket.addEventListener(WebSocketEvent.OPEN, onStatus);
			_websocket.addEventListener(WebSocketEvent.MESSAGE, onStatus);
			_websocket.addEventListener(WebSocketErrorEvent.CONNECTION_FAIL, function(event:WebSocketErrorEvent):void { 
				_websocket.close();
				onResult({err:event.text}); 
			});
			_websocket.connect();
		}
		
		public function onStatus(event:WebSocketEvent):void {
			
			switch(event.type) {
			case WebSocketEvent.OPEN:
				_websocket.sendUTF(_parameters);
				break;
			case WebSocketEvent.MESSAGE:
				var response:Object = JSON.parse(event.message.utf8Data);
				if (response[0][0]["var1"] != "value1" || response[0][0]["var2"] != "value2") {
					_websocket.close();
					onResult({err:"Unexpected response : "+event.message.utf8Data});
				} else if (_currentTest < NB_LOAD_TESTS) {
					_currentTest += 1;
					_websocket.sendUTF(_parameters); // Next request
				} else {
					_websocket.close();
					onResult({}); // Test Terminated!
				}
				break;
			case WebSocketEvent.CLOSED:
				break;
			}
		}
	}
}