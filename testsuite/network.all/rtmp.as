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
#include "../actionscript.all/dejagnu.as"

stop();

endOfTest = function()
{
	//note("END OF TEST");
//        check_totals(9);
    totals();
    play();
};

// -P FlashVars='hostname=localhost,rtmptport5080=rtmpport=1935'
if (hostname == undefined) {
    hostname="localhost";
    note("No hostname specified, defaulting to "+hostname);
}

if (rtmptport == undefined) {
    rtmptport = 5080;
    note("No RTMPT port specified, defaulting to "+rtmptport);
}

if (rtmpport == undefined) {
    rtmpport = 1935;
    note("No RTMP port specified, defaulting to "+rtmpport);
}

nc.statuses = new Array();
nc.onStatus = function()
{
    note('NetConnection.onStatus called with args: ');
};

function ResultHandler() {
    this.onResult = function(result) {
        note('default onResult called with args: '+dumpObject(arguments));
    };
//     this.onCustom = function(result) {
//         note('default onCustom called with args: '+dumpObject(arguments));
//     };
//     this.onDebugEvents = function(result) {
//         note('default onDebugEvents called with args: '+dumpObject(arguments));
//     };
//     this.onStatus = function(result) {
// 	note("default onStatus called with args: "+dumpObject(arguments));
//     };
};

//
// do the same thing for RTMP
//
ncrtmp = new NetConnection;
ncrtmp.statuses = new Array();
ncrtmp.onStatus = function()
{
    note('NetConnection.onStatus called with args: '+dumpObject(arguments));
    
//     lastStatusArgs = ncrtmp.statuses[ncrtmp.statuses.length-1];
//     if ((lastStatusArgs[0].level == "status") && (lastStatusArgs[0].code == "NetConnection.Connect.Success")) {
//         pass("RTMP connection - status Success");
//     } else {
//         fail("RTMP connection - status Success");
//     }
};

rtmpuri = "rtmp://"+hostname+":"+rtmpport+"/echo";
note("Connecting to "+rtmpuri);
ncrtmp.connect(rtmpuri);

// The network connection is not opened at connect() time, but when
// the first call() is made.
if (ncrtmp.isConnected == false) {
    pass("RTMP connection - isConnected");
} else {
    fail("RTMP connection - isConnected");
}

ncrtmp.onResult = function()
{
    note('NetConnection.onResult called with args: '+dumpObject(arguments));
    if (nc.isConnected == true) { // now it is connected
        pass("HTTP connection - status isConnected");
    } else {
        fail("HTTP connection - status isConnected");
    }
    if ((lastStatusArgs[0].level == "status") && (lastStatusArgs[0].code == "NetConnection.Connect.Success")) {
        pass("HTTP connection - status Success");
    } else {
        fail("HTTP connection - status Success");
    }
};

o=new ResultHandler();
ncrtmp.onResult = function()
{
    note("Got a result back from the  RTMP server.");
    
    // now it is connected
    if ((ncrtmp.isConnected == true) && (ncrtmp.statuses.length == 1)) {
        pass("RTMP connection - result");
    } else {
        fail("RTMP connection - result");        
    }
};

// This call starts the actual network connection
result1=false;;
o.onResult = function()
{
    note("Got a null result back from the RTMP server."+dumpObject(arguments));
    note(arguments[0]);
    if (arguments.length == 1) {
 	if (arguments[0] == null) {
	    result1=true;
 	}
    }
};
ncrtmp.call("echo", o, null);

// The network connection is not opened at connect() time, but should
// be now that we've made a ::call().
if (ncrtmp.isConnected == true) {
    pass("RTMP connection - isConnected");
} else {
    fail("RTMP connection - isConnected");
}

// // Empty String
// result3=false;
// tstr = new String();
// o=new ResultHandler();
// o.onResult = function()
// {
//     note("Got a string result back from the RTMP server."+dumpObject(arguments));
//     if (arguments.length == 1) {
// 	if (arguments[0].length == 0) {
// 	    result3 = true;
// 	}
//     }
// };
// ncrtmp.call("echo", o, tstr);

// Hello World!
result4=false;
tstr2 = "Hello World!";
o=new ResultHandler();
o.onResult = function()
{
    note("Got a string result back from the RTMP server."+dumpObject(arguments));
//     note("ARG4 is: " +dumpObject(arguments[0]));
    str = arguments[0].toString();
    if (arguments.length == 1) {
	if ((arguments[0].length == 12)
	    && (arguments[0].toString() == "Hello World!")) {
	    result4 = true;
	}
    }
};
ncrtmp.call("echo", o, tstr2);

// test1,test2,test3,test4

// Number 0
result5=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical 0 result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == 0) {
	    result5 = true;
	}
    }
};
ncrtmp.call("echo", o, 0);

// Number 1
result6=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical 1 result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == 1) {
	    result6 = true;
	}
    }
};
ncrtmp.call("echo", o, 1);

// Number -1
result7=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical -1 result back from the RTMP server."+dumpObject(arguments));
    if ((arguments.length == 1)) {
	note("FIXME: "+arguments[0].to_number());
	result7 = true;
    }
};
ncrtmp.call("echo", o, -1);

// Number 256
result8=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical 256 result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == 256) {
	    result8 = true;
	}
    }
};
ncrtmp.call("echo", o, 256);

// Number -256
result9=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical -256 result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == -256) {
	    result9 = true;
	}
    }
};
ncrtmp.call("echo", o, -256);

// Number 65536
result10=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical 65536 result back from the  RTMPserver."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == 65536) {
	    result10 = true;
	}
    }
};
ncrtmp.call("echo", o, 65536);

// Number -65536
result11=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical -65536 result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == -65536) {
	    result11 = true;
	}
    }
};
ncrtmp.call("echo", o, -65536);

// 1.5
result12=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical 1.5 result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == 1.5) {
	    result12 = true;
	}
    }
};
ncrtmp.call("echo", o, 1.5);

// -1.5
result13=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical -1.5 result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == -1.5) {
	    result13 = true;
	}
    }
};
ncrtmp.call("echo", o, -1.5);

// Number NaN
result14=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical NaN result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == NaN) {
	    result14 = true;
	}
    }
};
ncrtmp.call("echo", o, NaN);

// Number Infinity
result15=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got an numerical infinity result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == infinity) {
	    result15 = true;
	}
    }
};
ncrtmp.call("echo", o, infinity);

// Number -Infinity
result16=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got a numerical -infinity result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0] == -infinity) {
	    result16 = true;
	}
    }
};
ncrtmp.call("echo", o, -infinity);

// o=new ResultHandler();
// o.onResult = function()
// {
//     note("Got a result back from the server.");
//     check_equals(arguments.toString(), '1,two,true,4,5,6');
//     endOfTest();
// };
// ncrtmp.call("echo", o, 1, 'two', true, [4,5,6]);

// o=new ResultHandler();
// o.onResult = function()
// {
//     note("Got a result back from the server.");
//     check_equals(arguments.toString(), '1,2,3');
// };
// ncrtmp.call("echo", o, 1, 2, 3);


// Test empty array
tar = new Array();
result17=false;
o=new ResultHandler();
o.onResult = function()
{
    note("Got an empty array result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
	if (arguments[0].length == 0) {
	    result17 = true;
	}
    }
};
ncrtmp.call("echo", o, tar);

// Test array with only one item
result18=false;
tar = new Array();
tar.push(1);
o=new ResultHandler();
o.onResult = function()
{
    note("Got a single item array result back from the RTMP server."+dumpObject(arguments));
    if (arguments.length == 1) {
 	if (arguments[0][0] == 1) {
 	    result18 = true;
 	}
    }
};
ncrtmp.call("echo", o, tar);

// Test array with multiple items
result19=false;
tar = new Array();
tar.push(1);
tar.push(2);
tar.push(3);
o=new ResultHandler();
o.onResult = function()
{
    note("Got an 3 item array result back from the RTMP server."+dumpObject(arguments));
    if ((arguments.length == 1) && (arguments[0].length == 3)) {
	if ((arguments[0][0] == 1) && (arguments[0][1] == 2) && (arguments[0][2] == 3))  {
	    result19 = true;
	}
    }
//    note(arguments[0].toString());
//    check_equals(arguments[0].toString(), "1.2.3");
};
ncrtmp.call("echo", o, tar);

// Test sparse array
result20=false;
tar2 = new Array();
tar2.push(1);
tar2.push();
tar2.push();
tar2.push();
tar2.push(5);
o=new ResultHandler();
o.onResult = function()
{
    note("Got a sparse result back from the RTMP server."+dumpObject(arguments));
    if ((arguments.length == 1) && (arguments[0].length == 2)) {
	if ((arguments[0][0] == 1) && (arguments[0][1] == 5))  {
	    result20 = true;
	}   
    }
//    check_equals(arguments.toString(), "1..,,5");
};
ncrtmp.call("echo", o, tar2);

if (result1) {
    pass("RTMP: Echo NULL Object");
} else {
    fail("RTMP: Echo NULL Object");
}

if (result3) {
    pass("RTMP: Echo empty String");
} else {
    fail("RTMP: Echo empty String");
}

if (result4) {
    pass("RTMP: Echo short String");
} else {
    fail("RTMP: Echo short String");
}

if (result5) {
    pass("RTMP: Echo Number 0");
} else {
    fail("RTMP: Echo Number 0");
}

if (result6) {
    pass("RTMP: Echo Number 1");
} else {
    fail("RTMP: Echo Number 1");
}

if (result7) {
    pass("RTMP: Echo Number -1");
} else {
    fail("RTMP: Echo Number -1");
}
if (result8) {
    pass("RTMP: Echo Number 256");
} else {
    fail("RTMP: Echo Number 256");
}
if (result9) {
    pass("RTMP: Echo Number -256");
} else {
    fail("RTMP: Echo Number -256");
}
if (result10) {
    pass("RTMP: Echo Number 65536");
} else {
    fail("RTMP: Echo Number 65536");
}
if (result11) {
    pass("RTMP: Echo Number -65536");
} else {
    fail("RTMP: Echo Number -65536");
}
if (result12) {
    pass("RTMP: Echo Number 1.5");
} else {
    fail("RTMP: Echo Number 1.5");
}
if (result13) {
    pass("RTMP: Echo Number -1.5");
} else {
    fail("RTMP: Echo Number -1.5");
}
if (result14) {
    pass("RTMP: Echo Number NaN");
} else {
    fail("RTMP: Echo Number NaN");
}
if (result15) {
    pass("RTMP: Echo Number Infinity");
} else {
    fail("RTMP: Echo Number Infinity");
}
if (result16) {
    pass("RTMP: Echo Number -Infinity");
} else {
    fail("RTMP: Echo Number -Infinity");
}

if (result17) {
    pass("RTMP: Echo empty array");
} else {
    fail("RTMP: Echo empty array");
}
if (result18) {
    pass("RTMP: Echo 1 item array");
} else {
    fail("RTMP: Echo 1 item array");
}
if (result19) {
    pass("RTMP: Echo 3 item array");
} else {
    fail("RTMP: Echo 3 item array");
}
if (result20) {
    pass("RTMP: Echo sparse array");
} else {
    fail("RTMP: Echo sparse array");
}
