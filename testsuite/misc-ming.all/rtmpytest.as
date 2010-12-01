// This test may have similarities to the red5test, but we don't
// attempt to keep them in sync.

// Differences between red5 and rtmpy:
// rtmpy always returns an object: a single primitive type is
// convert to the corresponding object. If several arguments are
// sent, an array is returned. This array may contain primitive
// types.

note("SWF" + OUTPUT_VERSION + " - " + System.capabilities.version + "\n");
rcsid="red5test.as - <bzr revno here>";

#define info _root.note
#define note _root.note
#define fail_check _root.fail
#define pass_check  _root.pass
#define xfail_check _root.xfail
#define xpass_check _root.xpass

#include "../actionscript.all/check.as"
#include "../actionscript.all/utils.as"

stop();

endOfTest = function()
{
    // Should be incremented on connect
    check_equals(welcomecalls, 1);
    check_equals(connectcalls, 1);

    totals(70);
    trace("ENDOFTEST");
    play();
};

// -P FlashVars='hostname=localhost,rtmptport5080=rtmpport=1935'
hostname = RTMPY_HOST;

if (rtmpport == undefined) {
    rtmpport = 9984;
    note("No RTMP port specified, defaulting to "+rtmpport);
}
rtmpuri = "rtmp://"+hostname+":"+rtmpport+"/rtmpyecho";

test1 = function(nc)
{
    note("Running test 1");
    o = {};
    o.onResult = function(arg)
    {
        check_equals(arguments.length, 1);
        check_equals(arg.toString(), "hello");
        check_equals(typeof(arg), "object");
        test2(nc);
    };
    nc.call("echo", o, "hello");
};

test2 = function(nc)
{
    note("Running test 2");
    o = {};
    o.onResult = function(arg)
    {
        check_equals(arguments.length, 1);
        check_equals(typeof(arg), "object");
        check_equals(arg.toString(), "24");
        test3(nc);
    };
    nc.call("echo", o, 24);
};

// Send several arguments, get an array back.
test3 = function(nc)
{
    note("Running test 3");
    o = {};
    o.onResult = function(arg)
    {
        check_equals(arguments.length, 1);
        check_equals(typeof(arg), "object");
        check(arg.hasOwnProperty("length"));
        check_equals(arg.length, 3);

        check_equals(typeof(arg[0]), "number");

        check_equals(typeof(arg[1]), "object");
        check_equals(typeof(arg[1].x), "number");
        check_equals(arg[1].x, 23);
        check_equals(typeof(arg[1].y), "number");
        check_equals(arg[1].y, 67);
        check_equals(typeof(arg[1].text), "string");
        check_equals(arg[1].text, "a string");

        check_equals(typeof(arg[2]), "object");
        check_equals(arg[2].length, 5);

        test4(nc);
    };
    nc.call("echo", o, 24, { x:23, y:67, text:"a string" }, [ 1, 2, 3, 4, 5] );
};

// Send more things.
test4 = function(nc)
{
    note("Running test 4");
    o = {};
    o.onResult = function(arg)
    {
        check_equals(arguments.length, 1);
        check_equals(typeof(arg), "object");
        check(arg.hasOwnProperty("length"));
        check_equals(arg.length, 5);

        // It's a date.
        check_equals(arg[0].__proto__, Date.prototype);

        check_equals(arg[1], null);
        check_equals(typeof(arg[2]), "object");
        check_equals(arg[3], undefined);
        check_equals(arg[4], null);

        test5(nc);
    };
    nc.call("echo", o, new Date(0), new String(), {}, undefined, null);
};

test5 = function(nc)
{
    note("Running test 5");
    o = {};
    o.onResult = function(arg)
    {
        fail("onResult called when call failed");
    };

    nc.onStatus = function(obj) {
        trace(dumpObject(obj));
        check_equals(typeof(obj), "object");
        check(obj.hasOwnProperty("level"));
        check(obj.hasOwnProperty("code"));
        check(obj.hasOwnProperty("description"));
        check_equals(obj.level, "error");
        check_equals(obj.code, "NetConnection.Call.Failed");
        check_equals(obj.description, "Unknown method u'nonexistentfunc'");
        nc.onStatus = defaultOnStatus;
        test6(nc);
    };

    nc.call("nonexistentfunc", o, "hello", null);
};

test6 = function(nc)
{
    note("Running test 6");
    o = {};
    o.onResult = function(obj) {
        trace(dumpObject(obj));
        test7(nc);
    };

    nc.call("echo", o, 1);
};

test7 = function(nc)
{
    nc.onStatus = function(obj) {
        check_equals(obj.level, "status");
        check_equals(obj.code, "NetConnection.Connect.Closed");
        endOfTest();
    };
    nc.close();
};


runtests = function(nc)
{
    test1(nc);
};

ncrtmp = new NetConnection();
ncrtmp.statuses = new Array();
defaultOnStatus = function()
{
    this.statuses.push(arguments);
    note('NetConnection.onStatus called with args: ' + dumpObject(arguments));
    lastStatusArgs = ncrtmp.statuses[ncrtmp.statuses.length-1];
    if ((lastStatusArgs[0].level == "status") && (lastStatusArgs[0].code == "NetConnection.Connect.Success")) {
        pass("RTMP connection - status Success");
    } else {
        fail("RTMP connection - status Success");
    }
    runtests(this);
};

ncrtmp.onStatus = defaultOnStatus;

ncrtmp.disconnected = function(arg) {
    fail("boohoo");
};

welcomecalls = 0;
ncrtmp.welcome = function(arg) {
    ++welcomecalls;
    check_equals(arg[0].toString(), "You have connected!");
    trace(dumpObject(arg[1]));
};

connectcalls = 0;
ncrtmp.initial = function(arg) {
    ++connectcalls;
    check_equals(arg[0].toString(), "connection attempt received");
    o = arg[1];

    check(o.hasOwnProperty("fpad"));
    check_equals(typeof(o.fpad), "boolean");
    check_equals(o.fpad, false);

    check(o.hasOwnProperty("pageUrl"));
    check_equals(typeof(o.pageUrl), "undefined");
    check_equals(o.pageUrl, undefined);

    check(o.hasOwnProperty("videoFunction"));
    check_equals(typeof(o.videoFunction), "number");
    check_equals(o.videoFunction, 1);

    check(o.hasOwnProperty("tcUrl"));
    check_equals(typeof(o.tcUrl), "string");
    check_equals(o.tcUrl, rtmpuri);

    check(o.hasOwnProperty("app"));
    check_equals(typeof(o.app), "string");
    check_equals(o.app, "rtmpyecho");

    check(o.hasOwnProperty("flashVer"));
    check_equals(typeof(o.flashVer), "string");
    xcheck_equals(o.flashVer, "Our own special nonsense custom version");

    check(o.hasOwnProperty("audioCodecs"));
    check_equals(typeof(o.audioCodecs), "number");

    check(o.hasOwnProperty("videoCodecs"));
    check_equals(typeof(o.videoCodecs), "number");

    check(o.hasOwnProperty("swfUrl"));
    check_equals(typeof(o.swfUrl), "string");
    check_equals(o.swfUrl, _url);

    check(o.hasOwnProperty("capabilities"));
    check_equals(typeof(o.capabilities), "number");

    trace(dumpObject(arg[1]));
};

note("Connecting to "+rtmpuri);

$version = "Our own special nonsense custom version";
ncrtmp.connect(rtmpuri);

