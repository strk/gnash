
#include "../actionscript.all/check.as"

lc = new LocalConnection;
lc.connect("recv");


var i = 0;

stressTest = function() {
    var arg1 = i;
    var arg2 = { a:5, b:"A string, not too short, but also not really long" };
    var arg3 = new Array(i);
    lc.send("lc576", "stress", arg1, arg2, arg3);
};

lc.stressTestCheck = function(arg1, arg2, arg3) {
    check_equals(arg1, i);
    check_equals(typeof(arg2), "object");
    check_equals(arg2.a, 5);
    check_equals(arg2.b, "A string, not too short, but also not really long");
    check_equals(arg3.length, i);
    if (i < 1000) {
        ++i;
        stressTest();
    }
    else {
        endTests();
    };
    
};

endTests = function() {
    lc.send("lc576", "endTests");
};

runtests = function() {

    // This should not result in a call.
    lc.send("notaconnection", "nevercalled");

    // This should call the test1 function.
    lc.send("lc576", "test1");

    var a = 5;
    var b = false;
    var c = "A string";
    var d = new Date(0);
    var e = {};
    e.aa = 6;
    e.bb = 6;
    e.cc = 6;
    e.dd = 6;


    lc.send("lc576", "test2", a, b, c, d, e);
    
    var f = [1, "str", 6];
    
    lc.send("lc576", "test3", f);

    xml = new XML('<xml><t><t2 att="abob"><t3/></t2><t2><t3>hi</t3></t2></t></xml>');

    lc.send("lc576", "test4", xml);

    g = [];
    for (var i = 0; i < 150; ++i) {
       g.push("element" + i);
    };
    
    lc.send("lc576", "test5", g);

    // Not supported, should become undefined.
    xn = new XMLNode(1, "");
    check_equals(typeof(xn), "object");
    lc.send("lc576", "test6", xn);

    // Not supported, should become undefined.
    nc = new NetConnection;
    check_equals(typeof(nc), "object");
    lc.send("lc576", "test6", ns);

    // Not native, should be fine.
    c = new Color();
    check_equals(typeof(c), "object");
    lc.send("lc576", "test7", c);

    o = { a:5, b:"string" };
    e = { a:6, b:false };
    
    lc.send("lc576", "test8", o, e, o, e, o);

    stressTest();
};

getit = function()
{
    trace("Waiting for LC-Receive to reply.");
    lc.send("lc576", "ready");
};

// Wait until receiver is ready.
id = setInterval(getit, 1000);

lc.ready = function() {
    trace("LC-Receive is ready. Running tests");
    clearInterval(id);
    runtests();
};

// Called when LC-Send has finished. Exit in 2 seconds.
lc.finished = function() {
    trace("Received finish signal from LC-Receive. Exiting in 2 seconds");
    trace("ENDOFTEST");
    setInterval(exit, 2000);
};

exit = function() {
    loadMovie ("FSCommand:quit", "");
};

stop();
