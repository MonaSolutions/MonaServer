package RTMP
{
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.net.NetConnection;
	import flash.net.NetStream;
	import flash.utils.ByteArray;
	import flash.utils.Timer;
	
	public class RTMPp2p extends Test
	{
		private var _fullUrl:String;
		private var _nc1:NetConnection = null;
		private var _nc2:NetConnection = null;
		
		private var _tabNs:Array = new Array();
		
		private var _nbPublishers:int=0;
		private var _nbSubscribers:int=0;
		
		public function RTMPp2p(app:FunctionalTests, host:String, url:String)
		{
			super(app, "RTMPp2p", "Test p2p connections/deconnections between RTMP peers");
			_fullUrl = "rtmp://" + host + url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			// Connect the 2 peers
			_nc1 = new NetConnection();
			_nc1.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_nc1.connect(_fullUrl);
			_nc2 = new NetConnection();
			_nc2.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_nc2.connect(_fullUrl);
			
			_nbPublishers=0;
			_nbSubscribers=0;
		}
		
		// Proper finished test
		private function closeAll():void {
			_nc1.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_nc1.close();
			_nc1=null;
			_nc2.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_nc2.close();
			_nc2=null;
		}
		
		public function onStatus(event:NetStatusEvent):void {
			
			_app.INFO("onStatus : "+event.info.code);
			
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					
					// Begin publishing
					var publisher:NetStream = new NetStream(NetConnection(event.target));			
					publisher.addEventListener(NetStatusEvent.NET_STATUS, netStatusHandler);
					publisher.publish((event.target==_nc1)? "stream1" : "stream2");
					_tabNs.push(publisher);
					
					break;
				default: // Disconnection
					closeAll();
					onResult({err:"Unexpected event : "+event.info.code});
			}
		}
		
		private function netStatusHandler(event:NetStatusEvent):void {
			
			switch(event.info.code) {
				case "NetStream.Publish.Start":
					_nbPublishers++;
					
					// All publishers are ready? => play
					if (_nbPublishers==2) {
						_app.INFO("Subscribing to stream1 and stream2...");
						
						var subscriber1:NetStream = new NetStream(_nc1);			
						subscriber1.addEventListener(NetStatusEvent.NET_STATUS, netStatusHandler);
						subscriber1.play("stream2");
						_tabNs.push(subscriber1);
						
						var subscriber2:NetStream = new NetStream(_nc2);			
						subscriber2.addEventListener(NetStatusEvent.NET_STATUS, netStatusHandler);
						subscriber2.play("stream1");
						_tabNs.push(subscriber2);
					}
					break;
				case "NetStream.Play.Start":
					_nbSubscribers++;
					
					// All subscribtions OK? => subscribe a second time to stream1
					if (_nbSubscribers==2) {
						_app.INFO("Subscribing a second time to stream1...");
						var subscriber:NetStream = new NetStream(_nc2);			
						subscriber.addEventListener(NetStatusEvent.NET_STATUS, netStatusHandler);
						subscriber.play("stream1");
						_tabNs.push(subscriber);
					} else if (_nbSubscribers==3) {
						_app.INFO("Closing connection 2...");
						_nc2.close();
					}
					break;
				case "NetStream.Play.Failed":
					closeAll();
					if (_nbSubscribers==2) // Second subscription failed : Test OK!
						onResult({});
					else
						onResult({err:"Unexpected failed event (publishers : "+_nbPublishers+" ; subscribers : "+_nbSubscribers+")"});
					break;
				default:
					break;
			}
		}
	}
}