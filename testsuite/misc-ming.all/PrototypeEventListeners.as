// Build with:
//	makeswf -o PrototypeEventListeners.swf ../Dejagnu.swf PrototypeEventListeners.as
//

#include "../actionscript.all/check.as"
#include "../actionscript.all/utils.as"

#define info _root.note
#define note _root.note
#define fail_check _root.fail
#define pass_check  _root.pass
#define xfail_check _root.xfail
#define xpass_check _root.xpass

rcsid="$Id: PrototypeEventListeners.as,v 1.2 2008/02/13 14:44:14 bwy Exp $";


var countMC;

// Should remain 0;
var countTF = 0;

MovieClip.prototype.onMouseDown = function() { 
              note(this+".onMouseDown");
              check_equals(typeof(this), "movieclip");
              countMC++;
};

MovieClip.prototype.onKeyUp = function() { 
              note(this+".onKeyDown");
              note(typeof(this));
              countMC++;
};

TextField.prototype.onMouseDown = function() { 
              note(this+".onMouseDown");
              note(typeof(this));
              countTF++;
};

createEmptyMovieClip("clip1", 1);

with (clip1)
{
	lineStyle(4, 0);
	lineTo(50,50);
}

createEmptyMovieClip("clip2", 2);
with (clip2)
{
	lineStyle(4, 0);
	lineTo(50,25);
}

Dejagnu._y = 100;

// Tests

test1 = function()
{
	countMC = 0;
	note("1. Click!");
	_root.onMouseDown = function()
	{
		// clip1, clip2, and 2 Dejagnu clips.
		check_equals(countMC, 4);
		test2();
	};
};

test2 = function()
{
	countMC = 0;
	clip1.removeMovieClip();
	note("2. Click!");
	
	_root.onMouseDown = function()
	{
		// clip2 and 2 Dejagnu clips.
		check_equals(countMC, 3);
		endOfTest();
	};
};

endOfTest = function()
{
	check_totals(9);
	_root.ENDOFTEST = true;
	note("END OF TEST");
};

test1();

stop();
