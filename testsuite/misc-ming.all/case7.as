#include "check.as"

#define info _root.note
#define note _root.note
#define fail_check _root.fail
#define pass_check  _root.pass
#define xfail_check _root.xfail
#define xpass_check _root.xpass

stop();

if ( this == _root ) {
	createEmptyMovieClip("dejagnu", 1);
	dejagnu.loadMovie("Dejagnu.swf");

	createEmptyMovieClip("swf6", 2);
	swf6.loadMovie("case6.swf");

	onEnterFrame = function() {
		if ( swf6.loaded ) {
			delete onEnterFrame;
			check_equals(swf6.b, undefined); 
			check_equals(swf6.B, 'B'); 
			check_equals(swf6.a, 'a'); 
			check_equals(swf6.A, undefined); 
			check_equals(swf6.c, 'C'); 
			check_equals(swf6.C, undefined); 
			check_equals(swf6.d, undefined); 
			check_equals(swf6.D, 'd'); 
			totals(8);
		}
	};

} else {
	a='a';
	A='A';
	B='B';
	C='C';
	c='c';
	loaded=true;
}
