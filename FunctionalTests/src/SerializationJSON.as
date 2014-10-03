package
{
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;

	public class SerializationJSON extends Test
	{
		private var _host:String;
		private var _netConnection:NetConnection = null;
		
		public function SerializationJSON(app:FunctionalTests,host:String)
		{
			super(app, "SerializationJSON", "check toJSON and FromJSON validity");
			
			_host = host;
		}
		
		override public function run(onResult:Function):void {
			
			super.run(onResult);
			
			// make a new NetConnection and connect
			_netConnection = new NetConnection();
			_netConnection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_netConnection.connect("rtmfp://"+_host+"/FunctionalTests");
			_netConnection.client = this;
		}
		
		// Method called by Mona when finished
		public function onFinished(result:String):void { 
			_netConnection.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_netConnection.close();
			_onResult(result); 
		}
		
		// net status handler for the NetConnection
		private function onStatus(evt:NetStatusEvent):void {
			
			switch(evt.info.code) {
			case "NetConnection.Connect.Success":
				_netConnection.call("onSerialize",null,"json");
				break;
			default:
				_onResult(evt);
			}
		}
	}
}