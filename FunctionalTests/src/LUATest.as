package
{
	import flash.net.NetConnection;
	import flash.net.Responder;

	public class LUATest extends Test
	{
		private var _conn:NetConnection;
		private var _index:int;
		
		public function LUATest(app:FunctionalTests, testName:String, index:int, connection:NetConnection) {
			super(app, testName, testName+" LUA Test");
			_conn=connection;
			_index=index;
		}
		
		override public function run(onFinished:Function):void {
			
			super.run(onFinished);
			
			_conn.call("runTest", new Responder(onResult), _index);
		}
	}
}