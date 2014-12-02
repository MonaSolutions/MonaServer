package
{
	import flash.utils.getTimer;
	
	import mx.controls.TextArea;
	
	public class Test
	{
		private var _testName:String = null;
		private var _description:String = null;
		
		protected var _app:FunctionalTests = null;
		private var _onFinished:Function; // Async function to call when finished
		
		public var children:Array = null;
		
		private var _timeStart:int; // Time in msec when the test has been started
		private var _currentTest:int; // Current running test (for group of tests)
		
		// Static function to test equality of two objects (need to be run twice, the 2nd time with object2 in place of object1)
		public static function Equals(object1:Object, object2:Object):Boolean {
			
			for (var key:String in object1) {				
				if (!(object1[key] is int || object1[key] is String)) {
					if (!Equals(object1[key],object2[key]))
						return false;
				} else if(object1[key] != object2[key])
					return false;
			}
			return true;
		}
		
		public function Test(app:FunctionalTests, testName:String, description:String, isGroup:Boolean=false)
		{
			_testName = testName;
			_description = description;
			_app = app;
			
			if (isGroup)
				children = new Array();
		}
		
		public function run(onFinished:Function):void {
			
			_app.SUCCESS("Running " + label + " (" + description + ")...");
			_timeStart = getTimer();
			_onFinished = onFinished;
			
			// Run each child test
			_currentTest = 0;
			runNext();
		}
		
		// Run next test (for group of tests)
		public function runNext():Boolean {
			if (children != null && children.length > _currentTest) {
				var test:Test = children[_currentTest];
				test.run(onChildFinished);
				return true;
			}
			return false;
		}
		
		// Callback from a child of a group has ended
		protected function onChildFinished(test:Test, response:Object):void {
			_currentTest += 1;
			// Errror
			if (response.err) {
				_app.ERROR("Module " + label + " finished with errors.");
				_onFinished(this, response);
			} // Terminated?
			else if (!runNext()) {
				_app.SUCCESS("Module " + label + " OK (" + (getTimer()-_timeStart) + "ms)");
				_onFinished(this, response);
			}
		}
		
		// Test finished, print result and advise its parent
		protected function onResult(response:Object):void {
			
			if (response.err) 
				_app.ERROR(response.err);
			else
				_app.SUCCESS("Test " + label + " OK (" + ((response.elapsed!=null)? response.elapsed : getTimer()-_timeStart) + "ms)");
			
			_onFinished(this, response);
		}
		
		public function get label():String {
			return _testName;
		}
		
		public function get description():String {
			return _description;
		}
	}
}