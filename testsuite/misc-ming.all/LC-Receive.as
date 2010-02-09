#include "../actionscript.all/check.as"


lc = new LocalConnection;
lc.connect("lc576");

lc.ready = function() {
    trace("LC-Send is ready. Replying.");
    lc.send("recv", "ready");
};

lc.test1 = function()
{
    trace("Hello");
    pass();
};

lc.test2 = function(a, b, c, d, e, f)
{
    check_equals(typeof(a), "number");
    check_equals(a, 5);

    check_equals(typeof(b), "boolean");
    check_equals(b, false);

    check_equals(typeof(c), "string");
    check_equals(c, "A string");

    d1 = new Date(0);
    check_equals(typeof(d), "object");
    check_equals(d.toString(), d1.toString());

    check_equals(typeof(e), "object");
    check(e.hasOwnProperty("aa"));
    check_equals(typeof(e.aa), "number");
    check(e.hasOwnProperty("bb"));
    check(e.hasOwnProperty("cc"));
    check(e.hasOwnProperty("dd"));

};

lc.endTests = function() {
    totals();
};



stop();
