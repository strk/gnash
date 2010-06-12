/* 
getURL(url [, window [, "variables"]])

Parameters
    _self specifies the current frame in the current window.
    _blank specifies a new window.
    _parent specifies the parent of the current frame.
    _top specifies the top-level frame in the current window.

variables A GET or POST method for sending variables. If there are no
variables, omit this parameter. The GET method appends the variables to
the end of the URL, and is used for small numbers of variables. The POST
method sends the variables in a separate HTTP header and is used for
sending long strings of variables.

*/

var firstname = "Foo";
var lastname = "Bar";
var age = 100;
getURL("http://www.gnashdev.org", '_blank', 'POST');

getURL("http://www.gnashdev.org");
