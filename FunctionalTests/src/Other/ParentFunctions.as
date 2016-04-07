package Other
{
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.net.NetConnection;
	import flash.net.Responder;
	import flash.utils.Timer;
	
	import mx.controls.Alert;
	
	public class ParentFunctions extends Test
	{
		private var _host:String;
		private var _url:String;
		private var _connection:NetConnection;
	
		public function ParentFunctions(app:FunctionalTests, host:String, url:String)
		{
			super(app, "ParentFunctions", "Check heritage of sub applications");
			_host=host;
			_url=url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_connection = new NetConnection();
			_connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection.connect("rtmfp://" + _host + _url + "subapp/subsubapp");
		}
		
		// 1st response
		public function onRealNameApp(response:String):void { 
			if (response=="subsubapp") {
				_connection.call("getNameParentApp", new Responder(onNameParentApp)); 
			} else {
				onResult({err:"onRealNameApp : Expected 'subsubapp' and received '"+response+"'"});
				_connection.close();
			}
		}
		
		// 2nd response
		public function onNameParentApp(response:String):void { 
			if (response=="subapp") {
				// getNameApp doesn't exists in subsubApp => so it will fail
				_connection.call("getNameApp", new Responder(null, onErrorNameApp)); 
			} else {
				onResult({err:"onNameParentApp : Expected 'subapp' and received '"+response+"'"});
				_connection.close();
			}
		}
		
		// 3rd response
		public function onErrorNameApp(error:Object):void {
			if (error.description=="Method client 'getNameApp' not found in application /FunctionalTests/subapp/subsubapp") {
				_connection.call("getNameSuperParentApp", new Responder(onNameSuperParentApp)); 
			} else {
				onResult({err:"onErrorNameApp : Unexpected error log '"+error.description+"'"});
				_connection.close();
			}
		}
		
		// Last response
		public function onNameSuperParentApp(response:String):void {
			if (response=="FunctionalTests") {
				_connection.close();
				onResult({}); // Test Terminated!
			} else {
				onResult({err:"onNameSuperParentApp : Expected 'subapp' and received '"+response+"'"});
				_connection.close();
			}
		}
		
		public function onStatus(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					_connection.call("getRealNameApp", new Responder(onRealNameApp)); 
					break;
				case "NetConnection.Connect.Closed":
					break;
				default:
					onResult({err:event.info.code});
			}
		}

	}
}