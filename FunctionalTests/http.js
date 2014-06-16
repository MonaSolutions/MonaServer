
var FooMessage = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. \
Etiam posuere nisi neque. Mauris eu auctor quam, eget ornare sem. \
Nulla facilisi. Praesent non purus turpis. In vitae lectus imperdiet,\
condimentum eros eu, lobortis felis. Nunc massa lorem, congue sed mi quis, \
malesuada condimentum odio. Aliquam hendrerit tempus mauris sed lacinia. \
Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. \
Fusce feugiat sem est, ut tincidunt leo bibendum sed. Suspendisse rutrum sed mauris non auctor.";

function runHttpTimeoutTest() {
    
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open('POST', "", true); // asynchronous (to use timeout)
    xmlhttp.timeout = 1;
    
    // Treat the response
    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState != 4 || xmlhttp.status != 0)
            TEST_ERROR("HTTP", "timeout", "Timeout not reached (state : " + xmlhttp.readyState + ", status : " + xmlhttp.status + ")");
        else
            TEST_OK("HTTP", "timeout", 1);
    }
    // Send the POST request
    xmlhttp.setRequestHeader("Content-Type", "text/plain");
    xmlhttp.send(FooMessage);
}

function sendBruteForce(xmlhttp, count) {
    if (xmlhttp && (xmlhttp.readyState != 4 || xmlhttp.status != 200))
        TEST_ERROR("HTTP", "bruteforce", "HTTP response invalid (state : " + xmlhttp.readyState + ", status : " + xmlhttp.status + ", test " + count + ")");
    else {
        
        // Send 20 times
        if (count < 20) {
            try { 
                var newXhr = new XMLHttpRequest();
                if (count % 2 == 0)
                    newXhr.open('POST', "/FunctionalTests/", false); // synchronous
                else
                    newXhr.open('POST', "/FunctionalTests/subapp/", false); // synchronous
                newXhr.onreadystatechange = function () { sendBruteForce(newXhr, ++count); };
                newXhr.setRequestHeader("Content-Type", "text/plain");
                newXhr.send(FooMessage);
            } catch (ex) {
                TEST_ERROR("HTTP", "bruteforce", "HTTP error : " + ex);
            }
        } else
            TEST_OK("HTTP", "bruteforce", count);
    }
}

function runHttpBruteForceTest() {
      
    // Treat the response
    var count = 0;
    sendBruteForce(null, count);
}

// Add tests to the main list of tests
addTest("HTTP", "timeout", function() { runHttpTimeoutTest(); });
addTest("HTTP", "bruteforce", function() { runHttpBruteForceTest(); });