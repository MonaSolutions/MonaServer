package
{
	import mx.controls.TextArea;
	
	public class Test
	{
		private var _testName:String = null;
		private var _description:String = null;
		
		protected var _app:FunctionalTests = null;
		protected var _onResult:Function; // Async function to call when finished
		
		
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
		
		public function Test(app:FunctionalTests, testName:String, description:String)
		{
			_testName = testName;
			_description = description;
			_app = app;
		}
		
		public function run(onResult:Function):void {
			
			_onResult = onResult;
		}
		
		public function get testName():String {
			return _testName;
		}
		
		public function get description():String {
			return _description;
		}
	}
}