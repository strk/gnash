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

	createEmptyMovieClip("swf7", 2);
	swf7.loadMovie("case7.swf");

	onEnterFrame = function() {
		if ( swf7.loaded ) {
			delete onEnterFrame;
			check_equals(swf7.b, 'B');
			check_equals(swf7.B, 'B');
			check_equals(swf7.a, 'a');
			check_equals(swf7.A, 'a');
			check_equals(swf7.c, 'C');
			check_equals(swf7.C, 'C');

			check_equals(typeof(swf7.mca), 'movieclip');
			check_equals(typeof(swf7.mcA), 'movieclip');

			check_equals(typeof(swf7.mcb), 'movieclip');
			check_equals(typeof(swf7.mcB), 'movieclip');

			totals(10);
		}
	};
} else {
	createEmptyMovieClip("mcA", 3);
	createEmptyMovieClip("mcb", 4);
	a='a';
	B='B';
	c='c';
	C='C';
	D='D';
	d='d';
	loaded=true;
}

