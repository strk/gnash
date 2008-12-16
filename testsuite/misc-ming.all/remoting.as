//
// Build with:
//	makeswf -o remoting.swf ../Dejagnu.swf remoting.as
// Run with:
//	firefox DrawingApi.swf
// Or:
//	gnash DrawingApi.swf
//
//

#define info _root.note
#define note _root.note
#define fail_check _root.fail
#define pass_check  _root.pass
#define xfail_check _root.xfail
#define xpass_check _root.xpass

note("SWF" + OUTPUT_VERSION + " - " + System.capabilities.version + "\n");
rcsid="remoting.as - <bzr revno here>";

#include "../actionscript.all/check.as"
#include "../actionscript.all/utils.as"

endOfTest = function()
{
	//note("END OF TEST");
	check_totals(84);
};


if ( ! _root.hasOwnProperty('url') ) {
    // updated daily from bzr !
    // TODO: let ./configure specify another one
    url='http://www.gnashdev.org/testcases/remoting.php';
}

stop();

printInfo = function(result) {
	note("remote_port: " + result['remote_port']);
	note("request_id: " + result['request_id']);
	note("message: " + result['message']);
	note("arg1_type: " + result['arg1_type']);
	note("hex: " + result['hex']);
	//trace(result['message']);
};

handleOnStatus = function(result) {
	fail("server reported error. " + result);
};

nc = new NetConnection;
nc.onStatus = function()
{
	note('NetConnection.onStatus called with args: '+dumpObject(arguments));
};


function test1()
{
    note('Connecting to: '+url+' (pass "url" param to change)');
    nc.connect(url);

    o={onStatus:handleOnStatus};
    ary1=[1,2,3];
    nc.call("ary_123", o, ary1); // 31
    o.onResult = function(res) {
        //note(printInfo(res));
        connectionPort=res.remote_port;
        check_equals(res.request_id, '/1');
        check_equals(res.message, 'ary_123');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:03:00:3f:f0:00:00:00:00:00:00:00:40:00:00:00:00:00:00:00:00:40:08:00:00:00:00:00:00');
        test2();
    };
}

function test2()
{
    o={onStatus:handleOnStatus};
    ary2=[1,2,3]; ary2.custom='custom';
    nc.call("ary_123custom", o, ary2); // 32
    o.onResult = function(res) {
        //note(printInfo(res));
        check(res.remote_port != connectionPort);
        connectionPort = res.remote_port;
        check_equals(res.request_id, '/2');
        check_equals(res.message, 'ary_123custom');
        check_equals(res.arg1_type, 'ECMA_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:08:00:00:00:03:00:01:30:00:3f:f0:00:00:00:00:00:00:00:01:31:00:40:00:00:00:00:00:00:00:00:01:32:00:40:08:00:00:00:00:00:00:00:06:63:75:73:74:6f:6d:02:00:06:63:75:73:74:6f:6d:00:00:09');
    };

    o={onStatus:handleOnStatus};
    ary3=[1,2,3]; ary3.length=255;
    nc.call("ary_123length255", o, ary3); // 33
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/3');
        check_equals(res.message, 'ary_123length255');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:ff:00:3f:f0:00:00:00:00:00:00:00:40:00:00:00:00:00:00:00:00:40:08:00:00:00:00:00:00:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06');
    };

    o={onStatus:handleOnStatus};
    ary4=[]; ary4[3]=3;
    nc.call("ary__3", o, ary4); // 34
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/4');
        check_equals(res.message, 'ary__3');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:04:06:06:06:00:40:08:00:00:00:00:00:00');
        test5();
    };

    o={onStatus:handleOnStatus};
    ary5=[]; ary5['3']=3;
    nc.call("ary_s3", o, ary5); // 35
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/5');
        check_equals(res.message, 'ary_s3');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:04:06:06:06:00:40:08:00:00:00:00:00:00');
        test6();
    };

    o={onStatus:handleOnStatus};
    ary6=['0','0','0'];
    ary6.custom='custom'; AsSetPropFlags(ary6, 'custom', 1); // hide from enumeration
    nc.call("ary_000_assetpropflags", o, ary6); // 36
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/6');
        check_equals(res.message, 'ary_000_assetpropflags');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:03:02:00:01:30:02:00:01:30:02:00:01:30');
        test7();
    };
}

function test7()
{
    o={onStatus:handleOnStatus};
    ary7=[]; ary7['2.5']=1;
    nc.call("ary_float", o, ary7); // 37
    o.onResult = function(res) {
        //note(printInfo(res));
        check(res.remote_port != connectionPort);
        connectionPort = res.remote_port;
        check_equals(res.request_id, '/7');
        check_equals(res.message, 'ary_float');
        check_equals(res.arg1_type, 'ECMA_ARRAY');
        // The bug here is that gnash encodes 0 as the length of the array while
        // the expected behaviour is to encode 3.
        xcheck_equals(res.hex, '0a:00:00:00:01:08:00:00:00:03:00:03:32:2e:35:00:3f:f0:00:00:00:00:00:00:00:00:09');
    };

    o={onStatus:handleOnStatus};
    ary8=[]; ary8['256']=1;
    nc.call("ary_s256", o, ary8); // 38
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/8');
        check_equals(res.message, 'ary_s256');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:01:01:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:06:00:3f:f0:00:00:00:00:00:00');
    };

    o={onStatus:handleOnStatus};
    ary9=[]; ary9['-1']=1;
    nc.call("ary_sminus1", o, ary9); // 39
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/9');
        check_equals(res.message, 'ary_sminus1');
        check_equals(res.arg1_type, 'ECMA_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:08:00:00:00:00:00:02:2d:31:00:3f:f0:00:00:00:00:00:00:00:00:09');
    };

    o={onStatus:handleOnStatus};
    ary10=[]; ary10[-1]=1; // ECMA
    nc.call("ary_minus1", o, ary10);
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/10');
        check_equals(res.message, 'ary_minus1');
        check_equals(res.arg1_type, 'ECMA_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:08:00:00:00:00:00:02:2d:31:00:3f:f0:00:00:00:00:00:00:00:00:09');
        test11();
    };

    o={onStatus:handleOnStatus};
    ary11=['a','b','c']; // STRICT
    nc.call("ary_abc", o, ary11); // 
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/11');
        check_equals(res.message, 'ary_abc');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:03:02:00:01:61:02:00:01:62:02:00:01:63');
    };

    o={onStatus:handleOnStatus};
    ary12=[]; ary12['']=1; 
    nc.call("ary_emptypropname", o, ary12); //
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/12');
        check_equals(res.message, 'ary_emptypropname');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:00');
    };

    o={onStatus:handleOnStatus};
    ary13=[]; ary13[1] = ary11;
    nc.call("ary_nested", o, ary13); //
    o.onResult = function(res) {
        //note(printInfo(res));
        check_equals(res.remote_port, connectionPort);
        check_equals(res.request_id, '/13');
        check_equals(res.message, 'ary_nested');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:02:06:0a:00:00:00:03:02:00:01:61:02:00:01:62:02:00:01:63');
        test14();
    };
}

function test14()
{
    note('Connecting again to: '+url);
    nc.connect(url); // reconnect, should reset call id

    o={onStatus:handleOnStatus};
    ary13=[]; 
    nc.call("ary_newconnect", o, ary13); //
    o.onResult = function(res) {
        //note(printInfo(res));
        check(res.remote_port != connectionPort);
        connectionPort = res.remote_port;
        xcheck_equals(res.request_id, '/1'); // connection is reset
        check_equals(res.message, 'ary_newconnect');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:00');
    };

    note('Connecting again to: '+url);
    nc.connect(url); // reconnect, should reset call id

    o={onStatus:handleOnStatus};
    ary13=[]; 
    nc.call("ary_newconnect2", o, ary13); //
    o.onResult = function(res) {
        //note(printInfo(res));
        check(res.remote_port != connectionPort);
        connectionPort = res.remote_port;
        xcheck_equals(res.request_id, '/1'); // connection is reset
        check_equals(res.message, 'ary_newconnect2');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:00');
        onEnterFrame = test15;
    };
}

function test15()
{
    delete onEnterFrame;

    o={onStatus:handleOnStatus};
    ary13=[]; 
    nc.call("ary_newconnect", o, ary13); //
    o.onResult = function(res) {
        //note(printInfo(res));

        // connection ID is NOT reset if the call happens
        // on next frame
        check(res.remote_port != connectionPort);
        connectionPort = res.remote_port;
        xcheck_equals(res.request_id, '/2');
        check_equals(res.message, 'ary_newconnect');
        check_equals(res.arg1_type, 'STRICT_ARRAY');
        check_equals(res.hex, '0a:00:00:00:01:0a:00:00:00:00');
        test16();
    };
}

function test16()
{
    o={onStatus:handleOnStatus};
    nc.call("noarg", o); // no arguments
    o.onResult = function(res) {
        //note(printInfo(res));

        // connection ID is NOT reset if the call happens
        // on next frame
        check(res.remote_port != connectionPort);
        connectionPort = res.remote_port;
        xcheck_equals(res.request_id, '/3');
        check_equals(res.message, 'noarg');
        check_equals(res.arg_count, '0');
        check_equals(res.hex, '0a:00:00:00:00');
        test17();
    };
}

function test17()
{
    endOfTest();
}


// TODO: check encoding of calls w/out an handler, should
// have a request_id == '/' but we can't check immediately
// as we don't have the response handler....

test1();
