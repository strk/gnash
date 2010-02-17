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

lc.test4 = function(xml) {
    check_equals(xml.toString(),
        '<xml><t><t2 att="abob"><t3 /></t2><t2><t3>hi</t3></t2></t></xml>');
    check(xml instanceof XML);
};

lc.test5 = function(g) {
    check_equals(typeof(g), "object");
    check_equals(g.length, 150);
    check_equals(g[50], "element50");
    check(g instanceof Array);
};

// Unsupported object.
lc.test6 = function(f) {
    check_equals(typeof(f), "undefined");
};

lc.test7 = function(c) {
    check_equals(typeof(c), "object");
};

/// Object references.
// 
/// Our sender sends two object in alternation.
lc.test8 = function(arg1, arg2, arg3, arg4, arg5) {

    check_equals(typeof(arg1), "object");
    check_equals(typeof(arg2), "object");
    check_equals(typeof(arg3), "object");
    check_equals(typeof(arg4), "object");
    check_equals(typeof(arg5), "object");

    check_equals(arg1.a, 5);
    check_equals(arg2.a, 6);
    check_equals(arg3.a, 5);
    check_equals(arg4.a, 6);
    check_equals(arg5.a, 5);

    check_equals(arg1, arg3);
    check_equals(arg1, arg5);
    check_equals(arg2, arg4);

    check(arg1 != arg2);
    check(arg3 != arg4);
    check(arg4 != arg5);

};

var i = 0;

lc.stress = function(arg1, arg2, arg3) {
    check_equals(arg1, i);
    ++i;
    lc.send("recv", "stressTestCheck", arg1, arg2, arg3);
};

// Exit in 2 seconds.
lc.endTests = function() {
    totals();
    trace("Finished tests. Alerting LC-Send and exiting in 2 seconds");
    trace("ENDOFTEST");
    lc.send("recv", "finished");
    setInterval(exit, 2000);
};

exit = function() {
    loadMovie ("FSCommand:quit", "");
};

stop();
