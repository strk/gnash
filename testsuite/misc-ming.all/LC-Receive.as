#include "../actionscript.all/check.as"


lc = new LocalConnection;
lc.connect("lc576");

lc.ready = function() {
    trace("LC-Send is ready. Replying.");
    lc.send("recv", "ready");
};

lc.nevercall = function()
{
     fail_check("Function nevercall should never be called!");
};

lc.test1 = function()
{
    pass_check("Function test1 called");
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

lc.test3 = function(f) {
    check_equals(typeof(f), "object");
    check_equals(f.toString(), "1,str,6");
};

// Exit in 3 seconds.
lc.endTests = function() {
    totals();
    trace("Finished tests. Alerting LC-Send and exiting in 3 seconds");
    lc.send("recv", "finished");
    setInterval(exit, 3000);
};

exit = function() {
    loadMovie ("FSCommand:quit", "");
};

stop();
