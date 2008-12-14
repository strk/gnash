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

rcsid="remoting.as - <bzr revno here>";

#include "../actionscript.all/check.as"
#include "../actionscript.all/utils.as"

if ( ! _root.hasOwnProperty('url') ) {
	url='http://flash.jasonwoof.com/echo/echo.php';
}

stop();

note('Connecting to: '+url+' (pass "url" param to change)');

printInfo = function(result) {
	note("message: " + result['message']);
	note("type: " + result['type']);
	note("hex: " + result['hex']);
	//trace(result['message']);
};

handleOnStatus = function(result) {
	fail("server reported error. " + result);
};

endOfTest = function()
{
	//note("END OF TEST");
	check_totals(11);
};

nc = new NetConnection;
nc.onStatus = function()
{
	note('NetConnection.onStatus called with args: '+dumpObject(arguments));
};
nc.connect(url);

o={onStatus:handleOnStatus};
ary1=[1,2,3];
nc.call("ary_123", o, ary1); // 31
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'STRICT_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary2=[1,2,3]; ary2.custom='custom';
nc.call("ary_123custom", o, ary2); // 32
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'ECMA_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary3=[1,2,3]; ary3.length=255;
nc.call("ary_123length255", o, ary3); // 33
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'STRICT_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary4=[]; ary4[3]=3;
nc.call("ary__3", o, ary4); // 34
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'STRICT_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary5=[]; ary5['3']=3;
nc.call("ary_s3", o, ary5); // 35
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'STRICT_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary6=['0','0','0'];
ary6.custom='custom'; AsSetPropFlags(ary6, 'custom', 1); // hide from enumeration
nc.call("ary_000_assetpropflags", o, ary6); // 36
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'STRICT_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary7=[]; ary7['2.5']=1;
nc.call("ary_float", o, ary7); // 37
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'ECMA_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary8=[]; ary8['256']=1;
nc.call("ary_s256", o, ary8); // 38
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'STRICT_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary9=[]; ary9['-1']=1;
nc.call("ary_sminus1", o, ary9); // 39
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'ECMA_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary10=[]; ary10[-1]=1; // ECMA
nc.call("ayy_minus1", o, ary10);
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'ECMA_ARRAY');
	//check_equals(res.hex, 'xxx');
};

o={onStatus:handleOnStatus};
ary11=['a','b','c']; // STRICT
nc.call("ary_abc", o, ary11); // 
o.onResult = function(res) {
	//note(printInfo(res));
	check_equals(res.type, 'STRICT_ARRAY');
	//check_equals(res.hex, 'xxx');
	endOfTest();
};

