/**
 * API for Mona functionnals tests on web interfaces
 */

/** 
* Print a message on the viewer
* \param message to print
* \param style attribute of the 'div' tag that will contain the message
*/
function print(message, style) {

  var paragraph = document.createElement("div");
  var text = document.createTextNode(message);
  paragraph.appendChild(text);
  paragraph.setAttribute("style", style);
  divConsole.appendChild(paragraph);
}

// Macros for writing message on the result viewer
function ERROR(message) { print(message, "color:#FF0000;"); }
function NOTE(message) { print(message, "color:#00FF00;"); }
function INFO(message) { print(message, "color:#CCCCCC;"); }
function TEST_ERROR(name, subname, message) { ERROR("Test " + name + "::" + subname + " ERROR :"); ERROR(message); }
function TEST_OK(name, subname, count) { NOTE("Test " + name + "::" + subname + " OK (" + count + " tests)"); }

/** 
* HTTP Send message
*/
function send(message, contentType) {
  var xmlhttp = new XMLHttpRequest();
  xmlhttp.open('POST', "", false); // synchronous
  
  // Treat the response
  var result = "";
  xmlhttp.onreadystatechange = function () {
      if (xmlhttp.readyState == 4) {
          if (xmlhttp.status == 200) {
            
            result = xmlhttp.response;
            console.log(xmlhttp); // for chrome/firefox debug
          } else
            result = "code " + xmlhttp.status;
      }
  }
  // Send the POST request
  xmlhttp.setRequestHeader("Content-Type", contentType);
  xmlhttp.send(message);  
  
  return result;
}

/** 
* Erase text contained in console
*/
function eraseResult() {
  
  var childs = divConsole.getElementsByTagName("div");
  while (childs.length > 0) {
    divConsole.removeChild(childs[0]);
    childs = divConsole.getElementsByTagName("div");
  }
}

/** 
* Read a javascript file and call function at reception
* \param path2File relative path to the file .js to run
* \param func function to run at the end of the load
*/
function readJsScript(path2File/*, func*/) {

  var script = document.createElement('script');
  script.src = path2File;
  /*script.onload = func;*/
  document.getElementsByTagName('script')[0].parentNode.appendChild(script);
}

/**
* Add test to the the list remember function to call 
* Parameters are : title, subtitle, function
*/
function addTest() {
  if (arguments.length < 3) {
    alert("Bad number of arguments given to addTest (" + arguments.length + ")");
    return;
  }
  
  // Add option to the list of tests
  var testName = arguments[0] + "::" + arguments[1];
  var option = document.createElement("option");
  var text = document.createTextNode(testName);
  option.appendChild(text);
  option.setAttribute("value", testName);
  listTests.appendChild(option);
  option.selected = true; // selected by default
  
  // Add function to the map of available tests
  mapTest2Func[testName] = arguments[2];
}

/** 
* Function called when page is loaded completly
* Generate list of tests and select all
*/
function initList() {
  // Register html elements
  divConsole = document.getElementById("console");
  btRun = document.getElementById("btRun");
  listTests = document.getElementById("listTests");

  // Load tests files dynamically
  readJsScript("serialization.js");
  readJsScript("http.js");
}

/** 
* Main run function
*/
function run() {
  btRun.enabled = false;

  eraseResult();
  
  // Run selected tests
  for (var i = 0; i < listTests.options.length; i++) {
    if (listTests.options[i].selected) {
      // Run the test
      var testName = listTests.options[i].value;
      mapTest2Func[testName]();
    }
  }
}

var mapTest2Func = []; //! map of functions availables

// HTML elements
var divConsole; //! Console for writing results
var btRun;  //! Button for running tests
var listTests;  //! List of tests