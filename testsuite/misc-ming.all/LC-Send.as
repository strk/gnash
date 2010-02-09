

lc = new LocalConnection;
lc.connect("recv");

runtests = function() {

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

    var f = [];

    lc.send("lc576", "test2", a, b, c, d, e, f);
};

getit = function()
{
    trace("Hi");
    lc.send("lc576", "ready");
};

// Wait until receiver is ready.
//id = setInterval(lc, "send", 1000, "lc576", "ready");
id = setInterval(getit, 1000);

lc.ready = function() {
    trace("LC-Receive is ready. Running tests");
    clearInterval(id);
    runtests();
};

stop();
