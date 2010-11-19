// This test relies on a default deploy of red5 on localhost
//
// Build with:
//	makeswf -n network -o red5test.swf ../Dejagnu.swf red5test.as ../actionscript.all/dejagnu_so_fini.as
// Run with:
//	firefox red5test.swf
// Or:
//	gnash red5test.swf
//
//

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
    totals();
    play();
};

// -P FlashVars='hostname=localhost,rtmptport5080=rtmpport=1935'
hostname = RED5_HOST;

if (rtmpport == undefined) {
    rtmpport = 1935;
    note("No RTMP port specified, defaulting to "+rtmpport);
}

test1 = function(nc)
{
    note("Running test 1");
    o = {};
    o.onResult = function(arg)
    {
        check_equals(arguments.length, 1);
        check_equals(arg, "hello");
        check_equals(typeof(arg), "string");
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
        check_equals(typeof(arg), "number");
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

	endOfTest();
    };
    nc.call("echo", o, new Date(0), new String(), {}, undefined, null);
};

runtests = function(nc)
{
    test1(nc);
};

//
// do the same thing for RTMP
//
ncrtmp = new NetConnection();
ncrtmp.statuses = new Array();
ncrtmp.onStatus = function()
{
    this.statuses.push(arguments);
    note('NetConnection.onStatus called with args: '+dumpObject(arguments));
    lastStatusArgs = ncrtmp.statuses[ncrtmp.statuses.length-1];
    if ((lastStatusArgs[0].level == "status") && (lastStatusArgs[0].code == "NetConnection.Connect.Success")) {
        pass("RTMP connection - status Success");
    } else {
        fail("RTMP connection - status Success");
    }
    runtests(this);
};

rtmpuri = "rtmp://"+hostname+":"+rtmpport+"/echo";
note("Connecting to "+rtmpuri);
ncrtmp.connect(rtmpuri);

