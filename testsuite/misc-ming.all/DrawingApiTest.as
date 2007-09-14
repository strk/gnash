//
// Some tests for the Drawing API
// Build with:
//	makeswf -o DrawingApi.swf DrawingApi.as
// Run with:
//	firefox DrawingApi.swf
// Or:
//	gnash DrawingApi.swf
//

rcsid="$Id: DrawingApiTest.as,v 1.14 2007/09/14 16:32:38 strk Exp $";

#include "../actionscript.all/check.as"

printBounds = function(b)
{
	return ''+Math.round(b.xMin*100)/100+','+Math.round(b.yMin*100)/100+' '+Math.round(b.xMax*100)/100+','+Math.round(b.yMax*100)/100;
};

// Can draw both on a dynamically-created movie...
createEmptyMovieClip("a", 10);
// ... or on a statically-created one
//a = _root;

red = new Object();
red.valueOf = function() { return 0xFF0000; };

thick = new Object();
thick.valueOf = function() { return 20; };

halftransparent = new Object();
halftransparent.valueOf = function() { return 50; };

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


with (a)
{
	clear();

	bnd = printBounds(a.getBounds());
	if ( bnd == "6710886.35,6710886.35 6710886.35,6710886.35" ) {
		trace("PASSED: getBounds() returns "+bnd+" after clear");
	} else {
		trace("FAILED: getBounds() returns "+bnd+" after clear");
	}

	// The thick red line
	lineStyle(thick, red, 100);
	moveTo(100, 100);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "6710886.35,6710886.35 6710886.35,6710886.35");

	lineTo(200, 200);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "80,80 220,220");  // line is 20 pixels thick..

	// The hairlined horizontal black line
	lineStyle(0, 0x000000, 100);
	moveTo(220, 180);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "80,80 220,220");  // neither line style change nore moveTo change the bounds

	lineTo(280, 180);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "80,80 280,220");  // now hairlined line from 220,180 to 280,180 was added

	// The violet line
	moveTo(100, 200);
	lineStyle(5, 0xFF00FF, halftransparent);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "80,80 280,220"); // line style change and moveTo don't change anything

	lineTo(200, 250);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "80,80 280,255"); // line thinkness is now 5, so 250 gets to 255

	// The yellow line
	lineStyle(10, 0xFFFF00, 100);
	lineTo(400, 200);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "80,80 410,260"); // line thinkness of 10 adds to x (now 410) and to starting point y (was 250)

	// The green curve
	lineStyle(8, 0x00FF00, 100);
	curveTo(400, 120, 300, 100);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "80,80 410,260"); // the curve is all inside the current bounds

	// Transparent line
	lineStyle();
	lineTo(80, 100);

	// The black thick vertical line
	lineStyle(20);
	lineTo(80, 150);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "60,80 410,260"); // new thinkness of 20 moves our left margin

	// The ugly blue-fill red-stroke thingy
	moveTo(80, 180);
	lineStyle(2, 0xFF0000);
	beginFill(0x0000FF, 100);
	lineTo(50, 180);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "48,80 410,260"); // we get left to 50-thickness(2) now

	curveTo(20, 200, 50, 250);

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "18,80 410,260"); // we get left to 20-thickness(2) now

	lineTo(100, 250);
	lineTo(80, 180);
	endFill();
	lineTo(50, 150);

	// The clockwise blue-stroke, cyan-fill square
	moveTo(200, 100);
	lineStyle(1, 0x00FF00);
	beginFill(0x00FFFF, 100);
	lineTo(200, 120);
	lineTo(180, 120);
	lineTo(180, 100);
	lineTo(200, 100);
	endFill();

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "18,80 410,260"); // nothing new..

	// The counter-clockwise cyan-stroke, green-fill square
	moveTo(230, 100);
	lineStyle(1, 0x00FFFF);
	beginFill(0x00FF00, 50);
	lineTo(210, 100);
	lineTo(210, 120);
	lineTo(230, 120);
	lineTo(230, 100);
	endFill();

	// The clockwise green-stroke, violet-fill square, with move after beginFill
	lineStyle(1, 0x00FF00);
	beginFill(0xFF00FF, 100);
	moveTo(260, 100);
	lineTo(260, 120);
	lineTo(240, 120);
	lineTo(240, 100);
	lineTo(260, 100);
	endFill();

	// The green circle
	lineStyle(8, 0x000000);
	beginFill(0x00FF00, 100);
	drawCircle(a, 330, 160, 35);
	endFill();

	bnd = printBounds(a.getBounds());
	check_equals(bnd, "18,80 410,260"); // nothing new..
}

// Make the MovieClip "active" (grabbing mouse events)
// This allows testing of fill styles and "thick" lines
a.onRollOver = function() {};

frameno = 0;
a.createEmptyMovieClip("b", 2);
a.onEnterFrame = function()
{
	if ( ++frameno > 8 )
	{
		//this.clear();
		frameno = 0;
		ret = delete this.onEnterFrame;
		if ( ret ) {
			trace("PASSED: delete this.onEnterFrame returned "+ret);
		} else {
			trace("FAILED: delete this.onEnterFrame returned "+ret);
		}
	}
	else
	{
		this.b.clear();
		this.b.lineStyle(2, 0xFF0000);
		this.b.beginFill(0xFFFF00, 100);
		drawCircle(this.b, (50*frameno), 280, 10);
		this.b.endFill();
	}
};

createEmptyMovieClip("hitdetector", 3);
hitdetector.createEmptyMovieClip("shapeshape", 1);
with(hitdetector.shapeshape)
{
	lineStyle(2, 0x000000);
	beginFill(0xFFFF00, 100);
	drawCircle(hitdetector.shapeshape, 0, 0, 20);
	endFill();
}

hitdetector.createEmptyMovieClip("bboxpoint", 2);
with(hitdetector.bboxpoint)
{
	lineStyle(2, 0x000000);
	beginFill(0xFFFF00, 100);
	drawCircle(hitdetector.bboxpoint, 0, 0, 20);
	endFill();
	_x = 60;
}

hitdetector.createEmptyMovieClip("shapepoint", 3);
with(hitdetector.shapepoint)
{
	lineStyle(2, 0x000000);
	beginFill(0xFFFF00, 100);
	drawCircle(hitdetector.shapepoint, 0, 0, 20);
	endFill();
	_x = 120;
}

hitdetector._y = 350;


createEmptyMovieClip("cursor", 12);
with(cursor)
{
	lineStyle(2, 0x000000);
	beginFill(0xFF0000, 100);
	drawCircle(_root.cursor, 0, 0, 10);
	endFill();
	onEnterFrame = function()
	{
		hd = _root.hitdetector;

#if 0 // don't move the controls for now...
		with(hd)
		{
			if ( typeof(xshift) == 'undefined' )
			{
				xshift = 1;
			}
			else if ( xshift > 0 && _x >= 300 )
			{
				xshift = -1;
			}
			else if ( xshift < 0 && _x == 0 )
			{
				xshift = 1;
			}

			_x += xshift;
		}
#endif


		_x = _root._xmouse;
		_y = _root._ymouse;

		// Bounding box check
		if ( hitTest(_root.a) ) {
			hd.shapeshape._xscale=150;
			hd.shapeshape._yscale=150;
		} else {
			hd.shapeshape._xscale=100;
			hd.shapeshape._yscale=100;
		}

		// Bounding box check with circle center
		if ( _root.a.hitTest(_x, _y) ) {
			hd.bboxpoint._xscale=150;
			hd.bboxpoint._yscale=150;
		} else {
			hd.bboxpoint._xscale=100;
			hd.bboxpoint._yscale=100;
		}

		// Shape check with circle center
		if ( _root.a.hitTest(_x, _y, true) ) {
			hd.shapepoint._xscale=150;
			hd.shapepoint._yscale=150;
		} else {
			hd.shapepoint._xscale=100;
			hd.shapepoint._yscale=100;
		}
	};
}
