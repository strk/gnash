// This test relies on a default deploy of red5 on localhost
//
// Build with:
//        makeswf -n network -o red5test.swf ../Dejagnu.swf red5test.as ../actionscript.all/dejagnu_so_fini.as
// Run with:
//        firefox red5test.swf
// Or:
//        gnash red5test.swf
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
    totals(17);
    trace("ENDOFTEST");
};

// -P FlashVars='hostname=localhost,rtmptport5080=rtmpport=1935'
hostname = RED5_HOST;
funcId   = -1;

if (rtmpport == undefined) {
    rtmpport = 1935;
    note("No RTMP port specified, defaulting to "+rtmpport);
}

function checkAfter (netStream) {
    trace("Time :" + netStream.time);
    check(netStream.bufferLength > 0);
    check_equals(netStream.bufferTime, '3');
    xcheck(netStream.decodedFrames > 0);
    check(netStream.time > 0);
    check_equals(netStream.currentFps, '0' );
    clearInterval(funcId);
    endOfTest();
};
 
test1 = function(netStream) {
    note ("Running test1");
    check_equals(typeof(netStream), 'object');

    // Checking for properties
    xcheck_equals(typeof(netStream.audiocodec), 'number');
    xcheck_equals(typeof(netStream.videocodec), 'number');
    xcheck_equals(typeof(netStream.decodedFrames), 'number');
    check_equals(typeof(netStream.bytesTotal), 'number');
    check_equals(typeof(netStream.bytesLoaded), 'number');
    xcheck_equals(typeof(netStream.liveDelay), 'number');
    check_equals(typeof(netStream.bufferLength), 'number');
    check_equals(typeof(netStream.bufferTime), 'number');
    check_equals(typeof(netStream.currentFps), 'number');
    check_equals(typeof(netStream.time), 'number');

    netStream.play("avatar-vp6");
    netStream.setBufferTime(3);
    netStream.onStatus = function(object) {
        trace(object.code);
        trace(object.level);
        if(object.code == "NetStream.Play.Start") {
            funcId = setInterval(checkAfter, 1000, netStream);
        }
    };
};


runtests = function(ns)
{
    note("Running tests for NetStream.");
    test1(ns);
};

ncrtmp = new NetConnection();
ncrtmp.statuses = new Array();
ncrtmp.onStatus = function()
{
    this.statuses.push(arguments);
    note('NetConnection.onStatus called with args: ' + dumpObject(arguments));
    lastStatusArgs = ncrtmp.statuses[ncrtmp.statuses.length-1];
    if((lastStatusArgs[0].level == "status") && (lastStatusArgs[0].code == "NetConnection.Connect.Success")) {
        pass("RTMP connection - status Success");
        netStream = new NetStream(this);
        runtests(netStream);
    }
};

rtmpuri = "rtmp://"+hostname+":"+rtmpport+"/oflaDemo";
note("Connecting to "+rtmpuri);
ncrtmp.connect(rtmpuri);
