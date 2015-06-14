package RTMP
{
	import flash.events.AsyncErrorEvent;
	import flash.events.Event;
	import flash.events.IOErrorEvent;
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.media.Video;
	import flash.net.NetConnection;
	import flash.net.NetStream;
	import flash.net.Responder;
	import flash.utils.Timer;
	
	public class RTMPMedia extends Test {
	
		private var _protocol:String;
		private var _host:String;
		private var _url:String;
		
		private var _connection:NetConnection;
		private var _inStream:NetStream;
		private var _vid:Video = new Video(40,40);
		
		private var _dimensionChange:Boolean = false;
		
		private var _listMedias:Array = [ { name:"AudioAndVideo.flv", bytes:4922, isVideo:true }, 
											{ name:"VideoAndAudio.flv", bytes:4922, isVideo:true },
											{ name:"Audio.flv", bytes:2941, isVideo:false }, 
											{ name:"Video.flv", bytes:3373, isVideo:true } ]
		private var _indexMedia:int = 0;
		
		public function RTMPMedia(app:FunctionalTests, host:String, protocol:String, url:String) {
			
			super(app, protocol + "Media", "Check publishing media audio&video");
			_protocol = protocol;
			_host = host;
			_url = url;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_indexMedia = 0;
			runMediaTest();
		}
		
		private function runMediaTest():void {
			_app.INFO("Starting " + _listMedias[_indexMedia].name+" publication");
			
			// Prepare POST request
			_connection = new NetConnection();
			_connection.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection.connect(_protocol.toLocaleLowerCase() + "://" + _host + _url + "Media/");
			
			_dimensionChange = false;
			_indexMedia++;
		}
		
		
		// Manage NetStream events
		private function onStatusNetStream(evt:Event):void {
			 
			if (evt is IOErrorEvent)
				terminateConnection("NetStream error : " + IOErrorEvent(evt).text);
			else if (evt is AsyncErrorEvent)
				terminateConnection("NetStream error : " + AsyncErrorEvent(evt).text);
			else {
				 var event:NetStatusEvent = NetStatusEvent(evt);
				//_app.INFO("NetStream event : " + event.info.code);
				 
				 switch (event.info.code) {
					case "NetStream.Play.Start":
						_app.showVideo(_vid);
						_connection.call("readMedia", new Responder(onResponse, terminateConnection), _listMedias[_indexMedia-1].name);
						break;
					// Reset video size layout size
					case "NetStream.Video.DimensionChange":
						_dimensionChange = true;
						break;
				}
			}
		}
		
		public function onStatus(event:NetStatusEvent):void {
			//_app.INFO("onStatus : " + event.info.code);
			switch(event.info.code) {
				case "NetConnection.Connect.Success":
					_inStream = new NetStream(_connection);
					_inStream.addEventListener(NetStatusEvent.NET_STATUS, onStatusNetStream);
					_vid.attachNetStream(_inStream);
					_inStream.addEventListener(AsyncErrorEvent.ASYNC_ERROR, onStatusNetStream);
					_inStream.addEventListener(IOErrorEvent.IO_ERROR, onStatusNetStream);
					_inStream.play("file");
					break;
				default:
					terminateConnection(event.info.code);
			}
		}
		
		// Response handler of 'readMedia'
		// Continue publishing media or terminate connection
		private function onResponse(response:Object):void {
			
			if (response.err)
				terminateConnection(response.err);
			else if (response.finished) { // End of publication!
				var mediaObj:Object = _listMedias[_indexMedia - 1];
				
				if (_inStream.info.byteCount != mediaObj.bytes)
					terminateConnection("Unexpected number of bytes : "+_inStream.info.byteCount+ " (expected "+mediaObj.bytes+")");
				else if (mediaObj.isVideo && _dimensionChange == false)
					terminateConnection("NetStream.Video.DimensionChange has not been called");
				else
					terminateConnection(null);
			} else {
				var myTimer:Timer = new Timer(500, 1); // 500ms
				myTimer.addEventListener(TimerEvent.TIMER, function readAgain(e:Event):void { 
					_connection.call("readMedia", new Responder(onResponse, terminateConnection));
				});
				myTimer.start();
			}
		}
		
		// Close the connection
		private function terminateConnection(message:String):void {
			_connection.removeEventListener(NetStatusEvent.NET_STATUS, onStatus);
			_connection.close();
			_app.closeVideo();
			
			if (message)
				onResult({err:message}); // Error
			else if (_indexMedia < _listMedias.length)
				runMediaTest() // Next Media test
			else
				onResult({}); // End!
		}
		
	}
}