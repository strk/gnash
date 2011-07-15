//
// Build with:
//	makeswf -o DrawingApi.swf ../Dejagnu.swf DrawingApi.as
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

#ifdef LOADED_VERSION
#define SUPPRESS_RCSID_DUMP
#endif

#include "../actionscript.all/check.as"
#include "../actionscript.all/utils.as"

printBounds = function(b)
{
	return ''+Math.round(b.xMin*100)/100+','+Math.round(b.yMin*100)/100+' '+Math.round(b.xMax*100)/100+','+Math.round(b.yMax*100)/100;
};

// Draw a circle with given center and radius
// Uses 8 curves to approximate the circle
drawCircle = function (where, x, y, rad)
{
        var ctl = Math.sin(24*Math.PI/180)*rad;
        var cos = Math.cos(45*Math.PI/180)*rad;
        var sin = Math.sin(45*Math.PI/180)*rad;

        with (where)
        {
                moveTo(x, y-rad);
                curveTo(x+ctl, y-rad, x+cos, y-sin);
                curveTo(x+rad, y-ctl, x+rad, y);
                curveTo(x+rad, y+ctl, x+cos, y+sin);
                curveTo(x+ctl, y+rad, x, y+rad);
                curveTo(x-ctl, y+rad, x-cos, y+sin);
                curveTo(x-rad, y+ctl, x-rad, y);
                curveTo(x-rad, y-ctl, x-cos, y-sin);
                curveTo(x-ctl, y-rad, x, y-rad);
        }
};

dragOverHandler = function()
{
	check_equals(this, _root.target10);
	note(this+'.onDragOver called with args: '+dumpObject(arguments));
};
dragOutHandler = function()
{
	check_equals(this, _root.target10);
	note(this+'.onDragOver called with args: '+dumpObject(arguments));
};

 createTextField("tf", 0, 0, 100, 20, 5004);
tf.text = "Hello";

 createEmptyMovieClip("target10", 10);
with (target10)
{
	lineStyle(4, 0);
	beginFill(0xFF0000, 100);
	drawCircle(target10, 50, 50, 20);
#if 1
	target10.onDragOver = dragOverHandler;
	target10.onDragOut = dragOutHandler;
#else
	target10.onDragOver = function()
	{
		check_equals(this, _root.target10);
		note("(onDragOver): draggable dragged over "+this._droptarget);
	};
	target10.onDragOut = function()
	{
		check_equals(this, _root.target10);
		note("(onDragOut): draggable dragged out of "+this._droptarget);
	};
#endif
#ifdef LOADED_VERSION
	_x = 100;
#endif
};

createEmptyMovieClip("target20", 20);
with (target20)
{
	lineStyle(4, 0);
	beginFill(0x00FF00, 100);
	drawCircle(target20, 100, 50, 20);
#if 1
	target20.onDragOver = dragOverHandler;
	target20.onDragOut = dragOutHandler;
#else
	target20.onDragOver = function()
	{
		check_equals(this, _root.target20);
		note("(onDragOver): draggable dragged over "+this._droptarget);
	};
	target20.onDragOut = function()
	{
		check_equals(this, _root.target20);
		note("(onDragOut): draggable dragged out of "+this._droptarget);
	};
#endif

#ifdef LOADED_VERSION
	_x = 100;
#endif
};

createEmptyMovieClip("target100", 100);
with (target100)
{
	lineStyle(4, 0);
	beginFill(0x0000FF, 50);
	drawCircle(target100, 70, 100, 20);
#if 1
	target100.onDragOver = dragOverHandler;
	target100.onDragOut = dragOutHandler;
#else
	target100.onDragOver = function()
	{
		check_equals(this, _root.target100);
		note("(onDragOver): draggable dragged over "+this._droptarget);
	};
	target100.onDragOut = function()
	{
		check_equals(this, _root.target100);
		note("(onDragOut): draggable dragged out of "+this._droptarget);
	};
#endif

#ifdef LOADED_VERSION
	_x = 100;
#endif
};

#ifndef LOADED_VERSION

loadMovie("DragDropTestLoaded.swf", "_level50");

createEmptyMovieClip("loadedTarget", 30);
loadedTarget.loadMovie("DragDropTestLoaded.swf");
loadedTarget._x = 100;

createEmptyMovieClip("draggable50", 50);
with (draggable50)
{
	lineStyle(1, 0);
	beginFill(0x00FFFF, 50);
	drawCircle(draggable50, 0, 0, 10);
	draggable50.startDrag(true, 0, 0, 500, 120);
#if 1
	draggable50.onDragOver = dragOverHandler;
	draggable50.onDragOut = dragOutHandler;
#else
	draggable50.onDragOver = function()
	{
		check_equals(this, _root.draggable50);
		note("(onDragOver): draggable dragged over "+this._droptarget);
	};
	draggable50.onDragOut = function()
	{
		check_equals(this, _root.draggable50);
		note("(onDragOut): draggable dragged out of "+this._droptarget);
	};
#endif

	draggable50.lastDropTarget = undefined;
	draggable50.onEnterFrame = function()
	{
		//check_equals(this, _root.draggable50);
		if ( this._droptarget != this.lastDropTarget )
		{
			if ( this._droptarget != "" )
			{
				// reduces space on the textfield..
				//note(" -> draggable over "+this._droptarget);
			}
			this.lastDropTarget = this._droptarget;
		}
	};
};

Dejagnu._y = 100;

note("- This test is for drag&drop operations. Follow the instructions - ");

test1 = function()
{
	note("1. Click OUTSIDE of any drawing.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "");
		check_equals(eval(_root.draggable50._droptarget), undefined);
		test2();
	};
};

test2 = function()
{
	note("2. Click on the FIRST RED circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/target10");
		check_equals(eval(_root.draggable50._droptarget), _level0.target10);
		test3();
	};
};

test3 = function()
{
	note("3. Click on the FIRST GREEN circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/target20");
		check_equals(eval(_root.draggable50._droptarget), _level0.target20);
		test4();
	};
};

test4 = function()
{
	note("4. Click on the FIRST BLUE circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/target100");
		check_equals(eval(_root.draggable50._droptarget), _level0.target100);
		test5();
	};
};

test5 = function()
{
	note("5. Click on the SECOND RED circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "_level50/target10"); 
		check_equals(eval(_root.draggable50._droptarget), _level50.target10);
		test6();
	};
};

test6 = function()
{
	note("6. Click on the SECOND GREEN circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "_level50/target20"); 
		check_equals(eval(_root.draggable50._droptarget), _level50.target20);
		test7();
	};
};

test7 = function()
{
	note("7. Click on the SECOND BLUE circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "_level50/target100");
		check_equals(eval(_root.draggable50._droptarget), _level50.target100);
		test8();
	};
};

test8 = function()
{
	note("8. Click on the THIRD RED circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/loadedTarget/target10");
		check_equals(eval(_root.draggable50._droptarget), _level0.loadedTarget.target10);
		test9();
	};
};

test9 = function()
{
	note("9. Click on the THIRD GREEN circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/loadedTarget/target20");
		check_equals(eval(_root.draggable50._droptarget), _level0.loadedTarget.target20);
		test10();
	};
};

test10 = function()
{
	note("10. Click on the THIRD BLUE circle.");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/loadedTarget/target100");
		check_equals(eval(_root.draggable50._droptarget), _level0.loadedTarget.target100);

		// move the draggable over the first green square
		_root.draggable50._x = _root.draggable50._y = 50;

		test11();
	};
};

// Check that startDrag works for a textfield.
// The textfield should no longer be at 0, 100.
test11 = function()
{
	note("11. Click on the SECOND GREEN circle.");

    // Deliberately set target to null and then 
    // call stopDrag (old version) to make sure it still works.
    asm { push null settargetexpr };
    stopDrag();

    check_equals(_root.tf._x, 0);
    check_equals(_root.tf._y, 100);

    tf.startDrag = MovieClip.prototype.startDrag;
    tf.startDrag(true);
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/loadedTarget/target100");
        check(_root.tf._x != 0);
        check(_root.tf._y != 100);
		test12();
	};
};

test12 = function()
{
	note("12. Click ANYWHERE OUT of the THIRD BLUE circle (on another circle makes a better test)");
	_root.onMouseDown = function()
	{
		check_equals(_root.draggable50._droptarget, "/loadedTarget/target100");
		endOfTest(); // TODO: test that moving the draggable out of any drawing changes _droptarget to the empty string
	};
};


endOfTest = function()
{
	_root.ENDOFTEST = true;
	note("END OF TEST");
	check_totals(26);
	_root.onMouseDown = undefined;
};

test1(); // start the test


#endif // ndef LOADED_VERSION

stop();
