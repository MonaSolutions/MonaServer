package RTMFP
{
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	
	public class RTMFPLongQuery extends Test
	{
		private var _host:String;
		private var _url:String;
		private var _connection:NetConnection = null;
		
		private var _param1:String = "abcdefghijklmnopqrstuvwxyz";
		private var _param2:String = "é123456789é123456789é";
		private var _param3:String = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
		
		public function RTMFPLongQuery(app:FunctionalTests, host:String, url:String)
		{
			super(app, "RTMFPLongQuery", "Send long parameters in a the rtmfp url's query");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// 1) Create query and connect with RTMFP
			var newUrl:String = "rtmfp://" + _host + _url + "RTMFPLongQuery/?";
			newUrl += "param1=" + _param1;
			newUrl += "&param2=" + _param2;
			newUrl += "&param3=" + _param3;
			
			_connection = new NetConnection();
			_connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection.connect(newUrl);
		}
		
		private function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					_connection.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
					_connection.close();
					_connection = null;
					
					if (event.info.answer1 != _param1)
						onResult( { err:"answer1 is incorrect"} );
					else if (event.info.answer2 != _param2)
						onResult( { err:"answer2 is incorrect" } );
					else if (event.info.answer3 != _param3)
						onResult( { err:"answer3 is incorrect"} );
					else
						onResult( { } );
					break;
				default:
					_connection.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
					_connection.close();
					_connection = null;
					
					onResult({err:event.info.code});
			}
		}

	}
}