/**
 * Module for serialization/deserialization testing
 */
 
/**
* JSON Data for deserialization
*/
 var jsontestsdata = [
  {"msg":[{"x":10,"y":10,"width":100,"height":100}], "expected":[{"x":10,"y":10,"width":100,"height":100}]},
  {"msg":[[10,10,100,100]], "expected":[[10,10,100,100]]},
  {"msg":[[{"x":10,"y":10},100,100]], "expected":[[{"x":10,"y":10},100,100]]},
  {"msg":[[100,100,{"x":10,"y":10}]], "expected":[[100,100,{"x":10,"y":10}]]},
  {"msg":[[{"x":[100,100],"y":[10,10]}]], "expected":[[{"x":[100,100],"y":[10,10]}]]},
  {"msg":[[{"x":[{"y":[10,10]},15]},25]], "expected":[[{"x":[{"y":[10,10]},15]},25]]}
 ];
 
 /**
  * XML Data for deserialization
  */
var xmltestsdata = [
  {"msg":"<__array><__array><x/></__array></__array>", "expected":"<__array><__array><x/></__array></__array>"},
  {"msg":'<__array><__array><x x1="1" x2="2" x3="3"/></__array></__array>', "expected":"<__array><__array><x><x2>2</x2><x1>1</x1><x3>3</x3></x></__array></__array>"}, // Warn : order can differ
  {"msg":'<__array><__array><x x1="1" x2="2" x3="3"><y>10</y></x></__array></__array>', "expected":"<__array><__array><x><x2>2</x2><x1>1</x1><y>10</y><x3>3</x3></x></__array></__array>"},  // Warn : order can differ
  {"msg":"<__array><__array><y>10</y><x>10</x><height>100</height><width>100</width></__array></__array>","expected":"<__array><__array><y>10</y><x>10</x><height>100</height><width>100</width></__array></__array>"},
  {"msg":"<__array><__array><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></__array></__array>","expected":"<__array><__array><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></__array></__array>"},
  {"msg":"<__array><__array><__noname>10</__noname><__noname>10</__noname><__noname>100</__noname><__noname>100</__noname></__array></__array>","expected":"<__array><__array><__noname>10</__noname><__noname>10</__noname><__noname>100</__noname><__noname>100</__noname></__array></__array>"},
  {"msg":"<__array><__array><a>10</a><a>10</a><a>100</a><a>100</a></__array></__array>", "expected":"<__array><__array><a>10</a><a>10</a><a>100</a><a>100</a></__array></__array>"},
  {"msg":"<__array><__array><x/><x>10</x></__array></__array>", "expected":"<__array><__array><x/><x>10</x></__array></__array>"},
  {"msg":"<__array><__array><a>100</a><a>100</a><a><y>10</y></a><a><x>10</x></a></__array></__array>", "expected":"<__array><__array><a>100</a><a>100</a><a><y>10</y></a><a><x>10</x></a></__array></__array>"},
  {"msg":"<__array><__array><a><y>10</y></a><a><x>10</x></a><a>100</a><a>100</a></__array></__array>", "expected":"<__array><__array><a><y>10</y></a><a><x>10</x></a><a>100</a><a>100</a></__array></__array>"},
  {"msg":"<__array><__array><a><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></a></__array></__array>", "expected":"<__array><__array><a><y>10</y><x>10</x><__noname>100</__noname><__noname>100</__noname></a></__array></__array>"},
  {"msg":"<__array><__array><a><y>10</y><x>10</x></a><a>100</a><a>100</a></__array></__array>", "expected":"<__array><__array><a><y>10</y><x>10</x></a><a>100</a><a>100</a></__array></__array>"},
  {"msg":"<__array><__array><a><y>20</y><x>10</x>30</a></__array></__array>", "expected":"<__array><__array><a><y>20</y><x>10</x><__noname>30</__noname></a></__array></__array>"},
  {"msg":"<__array><__array><a><__array><__noname>100</__noname><__noname>100</__noname></__array></a><a>10</a><a>10</a></__array></__array>", "expected":"<__array><__array><a><__array><__noname>100</__noname><__noname>100</__noname></__array></a><a>10</a><a>10</a></__array></__array>"}
];

/** 
* Return true if object1 is  object 2
*/
function equals(object1, object2) {
    
    // First we check size of objects
    if (Object.keys(object1).length != Object.keys(object2).length)
        return false;
    
    // Then we check each elements
    for (var prop in object1) {
        if (typeof object1[prop] === 'object') {
            // Recursive call
            if (!equals(object1[prop], object2[prop]))
                return false;
        } else if (object1[prop] != object2[prop])
                return false;
    }
    
    return true;
}

/** 
* Run one test
* \param name Name of the Test
* \param data Matrix containing data to send and result expected
* \param mode type of data
*/
function runDeserializeTest(mode, data) {
  
  for (var i=0; i < data.length; i++) {
  
    var strExpected = data[i].expected;
    var isOK = false;
    var result;
    if (mode == "xml") {
        result = send(data[i].msg, "text/xml");
        isOK = data[i].expected == result;
    }
    else {// JSON
        result = send(JSON.stringify(data[i].msg), "application/json");
        isOK = equals(data[i].expected, JSON.parse(result));
        strExpected = JSON.stringify(data[i].expected);
    }
    
    if(!isOK) {
    
      var error = "Test[" + i + "] : '" + strExpected + "' expected and '" + result + "' received";
      TEST_ERROR("Deserialization", mode, error);
      return;
    }
  }
  TEST_OK("Deserialization", mode, data.length);
}

/** 
* Run tests of serialization sending commands to lua application
* (these tests are included in lua script)
* \param mode type of serialization requested
*/
function runSerializationTests(mode) {

  var result = "";
  var i = 0;
  do {
  
    result = send("<__array><__noname>onSerialize</__noname><__noname>" + mode + "</__noname></__array>", "text/xml");
    i++;
  } while (result == "continue");
  
  if (result == "terminate")
    TEST_OK("Serialization", mode, i);
  else
    TEST_ERROR("Serialization", mode, result);
}

// Add tests to the main list of tests
addTest("Serialization", "json", function() { runSerializationTests("json"); });
addTest("Serialization", "xml", function() { runSerializationTests("xml"); });
addTest("Deserialization", "json", function() { runDeserializeTest("json", jsontestsdata); });
addTest("Deserialization", "xml", function() { runDeserializeTest("xml", xmltestsdata); });