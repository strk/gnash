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

#define info _root.note
#define note _root.note
#define fail_check _root.fail
#define pass_check  _root.pass
#define xfail_check _root.xfail
#define xpass_check _root.xpass

note("SWF" + OUTPUT_VERSION + " - " + System.capabilities.version + "\n");
rcsid="red5test.as - <bzr revno here>";

#include "../actionscript.all/check.as"
#include "../actionscript.all/utils.as"

stop();

endOfTest = function()
{
	//note("END OF TEST");
	check_totals(9);
	play();
};

if ( ! _root.hasOwnProperty('host') ) {
    host=RED5_HOST;
}

nc = new NetConnection;
nc.statuses = new Array();
nc.onStatus = function()
{
    this.statuses.push(arguments);
	note('NetConnection.onStatus called with args: '+dumpObject(arguments));
};

function ResultHandler() {
    this.onResult = function(result) {
        note('default onResult called with args: '+dumpObject(arguments));
    };
    this.onCustom = function(result) {
        note('default onCustom called with args: '+dumpObject(arguments));
    };
    this.onDebugEvents = function(result) {
        note('default onDebugEvents called with args: '+dumpObject(arguments));
    };
    this.onStatus = function(result) {
	note("default onStatus called with args: "+dumpObject(arguments));
    };
};

// nc.onStatus: level:error, code:NetConnection.Connect.InvalidApp
// nc.onStatus: level:status, code:NetConnection.Connect.Closed
//nc.connect("rtmp://localhost/");

// nc.onStatus: level:status, code:NetConnection.Connect.Success
nc.connect("rtmp://localhost/echo");
check_equals(nc.isConnected, false); // not yet
check_equals(nc.statuses.length, 0);

o=new ResultHandler();
o.onResult = function()
{
    check_equals(nc.isConnected, true); // now it is connected
    check_equals(nc.statuses.length, 1);
    lastStatusArgs = nc.statuses[nc.statuses.length-1];
    check_equals(lastStatusArgs[0].level, 'status');
    check_equals(lastStatusArgs[0].code, 'NetConnection.Connect.Success');
	check_equals(arguments.toString(), '1');
};
nc.call("echo", o, 1);

o=new ResultHandler();
o.onResult = function()
{
	check_equals(arguments.toString(), '1,2,3');
};
nc.call("echo", o, 1, 2, 3);

o=new ResultHandler();
o.onResult = function()
{
	check_equals(arguments.toString(), '1,two,true,4,5,6');
    endOfTest();
};
nc.call("echo", o, 1, 'two', true, [4,5,6]);
