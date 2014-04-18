//
// Some tests for the Drawing API
// Build with:
//	makeswf -o DrawingApi.swf DrawingApi.as
// Run with:
//	firefox DrawingApi.swf
// Or:
//	gnash DrawingApi.swf
//
//
// Click the mouse button to turn the cursor shape into a mask and back.
// Press a number on the keyboard to switch between "pages" of the drawing.
// We currently have pages 1, 2, 3, 4.
// All pages are tested automatically.
//
// '-' and '+' decrement and increment _alpha
// 'h' toggles _visible
//


#include "../actionscript.all/check.as"

// Make Matrix visible for easier gradient tests.
ASSetPropFlags(_global, "flash", 0, 5248);

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
	check( ! hitTest(376, 180, true) ); 
	check( hitTest(376, 139, true) );

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

	// The counter-clockwise cyan-stroke, green-fill (alpha 50) square
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

	createEmptyMovieClip("zshape", 1);
	with (zshape)
	{
		// The Z-shaped 3 edges stroke 
		//
		// 
		//   0-----1--+
		//            |
		//            2
		//            |
		//            |
		//            +--3----4
		//
		lineStyle(4, 0);
		x = 223; y=145;
		moveTo(x, y);
		lineTo(x+20, y+0);
		lineTo(x+20, y+20);
		lineTo(x+50, y+20);
	}
	bnd = printBounds(zshape.getBounds());
	check_equals(bnd, "219,141 277,169");
	//                "223,145 273,165" + 4 for thickness
	check_equals( typeof(zshape.hitTest), 'function' );
	check( zshape.hitTest(223, 145, true) ); // 0
	check( zshape.hitTest((223+10), (145+0), true) ); // 1
	check( zshape.hitTest((223+20), (145+5), true) ); // 2
	check( zshape.hitTest((223+25), (145+20), true) ); // 3
	check( zshape.hitTest(273, 165, true) ); // 4

	// The ugly orange stroke / light brown fill
	// expected to auto-close on endFill()
	createEmptyMovieClip("x", 20);
	with (x)
	{
		lineStyle(6, 0xFF8800);
		beginFill(0x888800, 100);
		moveTo(100, 100);
		//lineTo("string", NaN); // equivalent to 0, 0
		lineTo(0, 0);
		//curveTo(100, "string", Object, 100); // equivalent to 100, 0, 0, 100
		curveTo(100, 0, 0 ,100);
		//moteTo(undefined, 0); // equivalent to 0, 0
		//moveTo(0,0); ???
		//lineTo(100, null); // equivalent to 100, 0
		lineTo(100, 0);
		endFill();
	}
	x._x = 100;
	x._y = 150;
	x._xscale = 30;
	x._yscale = 30;

	check( x.hitTest(126, 168, true) ); 
	check( x.hitTest(112, 158, true) ); 

    // COMPATIBLITY NOTE:
    // flashplayer 10.0.12.10 for linux fails this, whereas 9.0.115.0 doesn't
	check( x.hitTest(122, 157, true) );

	check( ! x.hitTest(120, 155, true) ); 
	check( x.hitTest(120, 155, false) ); 
	check( ! x.hitTest(116, 177, true) ); 
	check( x.hitTest(116, 177, false) ); 
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
                // #40315: test that we don't crash without a line style.
                this.b.lineTo(10, 20);

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

//---------------------------------------------------------------------------
// Some invalid shapes, get there hitting the right-arrow
//---------------------------------------------------------------------------

createEmptyMovieClip("inv", 100);
with(inv)
{
	// Crossing edge
	createEmptyMovieClip("inv1", 1);
	with (inv1)
	{
		moveTo(10, 10);
		beginFill(0xFF0000);
		lineTo(20, 10);
		lineTo(10, 20);
		lineTo(20, 20);
		endFill(); // should close to 10,10
	}
	inv1._xscale = inv1._yscale = 400;
	inv1.onRollOver = function() {};

	// Four intersecting edges  (like in "four in a row")
	createEmptyMovieClip("inv2", 2);
	with (inv2)
	{
		lineStyle(0, 0); // hairline
		beginFill(0x00FF00);

		moveTo(10, 8);
		lineTo(10, 20);

		moveTo(8, 10);
		lineTo(20, 10);

		moveTo(18, 8);
		lineTo(18, 20);

		moveTo(8, 18);
		lineTo(20, 18);

	}
	inv2._xscale = inv2._yscale = 400; inv2._x = 100;
	inv2.onRollOver = function() {};

	// Opposite el shapes
	createEmptyMovieClip("inv3", 3);
	with (inv3)
	{
		lineStyle(0, 0); // hairline
		beginFill(0x00FF00);

		moveTo(10, 8);
		lineTo(10, 20);
		lineTo(2, 20);

		moveTo(18, 8);
		lineTo(18, 20);
		lineTo(26, 20);

	}
	inv3._xscale = inv3._yscale = 400; inv3._y = 100;
	inv3.onRollOver = function() {};

	check(  inv3.hitTest((8*4), 100 + (18*4), true)  );  // Inside the left autofill
	check( !inv3.hitTest((4*4), 100 + (10*4), true)  );  // Outside the left autofill (but in the boundaries)
	check(  inv3.hitTest((4*4), 100 + (10*4), false) );  // Outside the left autofill (but in the boundaries)

	check(  inv3.hitTest((20*4), 100 + (18*4), true)  );  // Inside the right autofill
	check( !inv3.hitTest((24*4), 100 + (10*4), true)  );  // Outside the right autofill (but in the boundaries)
	check(  inv3.hitTest((24*4), 100 + (10*4), false) );  // Outside the right autofill (but in the boundaries)

	// Nested squares (inner is an hole)
	// Both squares are defined in counterclockwise order
	createEmptyMovieClip("inv4", 4);
	with (inv4)
	{
		lineStyle(0, 0); // hairline
		beginFill(0x00FF00);

		moveTo(10, 10);
		lineTo(10, 20);
		lineTo(20, 20);
		lineTo(20, 10);
		lineTo(10, 10);

		moveTo(12, 12);
		lineTo(12, 18);
		lineTo(18, 18);
		lineTo(18, 12);
		lineTo(12, 12);
	}
	inv4._xscale = inv4._yscale = 400;
	inv4._y = 100; inv4._x = 100;
	inv4.onRollOver = function() {};

	// check that a point inside the hole doesn't hit the shape
	// (gnash fails due to bogus point_test, or missing normalization)
	check( ! inv4.hitTest(100 + (15*4), 100 + (15*4), true) ); 

	// while a points on the border do hit it
	check( inv4.hitTest(100 + (11*4), 100 + (11*4), true) );  // Upper-Left
	check( inv4.hitTest(100 + (11*4), 100 + (14*4), true) );  // Center-Left
	check( inv4.hitTest(100 + (11*4), 100 + (19*4), true) );  // Lower-Left
	check( inv4.hitTest(100 + (14*4), 100 + (19*4), true) );  // Lower-Center
	check( inv4.hitTest(100 + (19*4), 100 + (19*4), true) );  // Lower-Right
	check( inv4.hitTest(100 + (19*4), 100 + (14*4), true) );  // Center-Right
	check( inv4.hitTest(100 + (19*4), 100 + (11*4), true) );  // Upper-Right
	check( inv4.hitTest(100 + (14*4), 100 + (11*4), true) );  // Upper-Center

	// Nested squares (inner is an hole)
	// Outer square counterclockwise order, inner in closwise order
	// NOTE that there's no difference with inv4 in rendering and hit test
	createEmptyMovieClip("inv5", 5);
	with (inv5)
	{
		lineStyle(0, 0); // hairline
		beginFill(0x00FF00);

		moveTo(10, 10);
		lineTo(10, 20);
		lineTo(20, 20);
		lineTo(20, 10);
		lineTo(10, 10);

		moveTo(12, 12);
		lineTo(18, 12);
		lineTo(18, 18);
		lineTo(12, 18);
		lineTo(12, 12);
	}
	inv5._xscale = inv5._yscale = 400;
	inv5._y = 100; inv5._x = 150;
	inv5.onRollOver = function() {};

	// check that a point inside the hole doesn't hit the shape
	// (gnash fails due to bogus point_test, or missing normalization)
	check( ! inv5.hitTest(150 + (15*4), 100 + (15*4), true) ); 

	// while a points on the border do hit it
	check( inv5.hitTest(150 + (11*4), 100 + (11*4), true) );  // Upper-Left
	check( inv5.hitTest(150 + (11*4), 100 + (14*4), true) );  // Center-Left
	check( inv5.hitTest(150 + (11*4), 100 + (19*4), true) );  // Lower-Left
	check( inv5.hitTest(150 + (14*4), 100 + (19*4), true) );  // Lower-Center
	check( inv5.hitTest(150 + (19*4), 100 + (19*4), true) );  // Lower-Right
	check( inv5.hitTest(150 + (19*4), 100 + (14*4), true) );  // Center-Right
	check( inv5.hitTest(150 + (19*4), 100 + (11*4), true) );  // Upper-Right
	check( inv5.hitTest(150 + (14*4), 100 + (11*4), true) );  // Upper-Center

	// Nested squares (this time we call beginFill again before the move)
	// Outer square counterclockwise order, inner in clockwise order
	// This time, the inner square will NOT be considered an hole !
	createEmptyMovieClip("inv6", 6);
	with (inv6)
	{
		lineStyle(0, 0); // hairline
		beginFill(0x00FF00);

		moveTo(10, 10);
		lineTo(10, 20);
		lineTo(20, 20);
		lineTo(20, 10);
		lineTo(10, 10);

		// this forces endFill call, which triggers finalization of previous path (I think)
		beginFill(0x00FF00);

		moveTo(12, 12);
		lineTo(18, 12);
		lineTo(18, 18);
		lineTo(12, 18);
		lineTo(12, 12);
	}
	inv6._xscale = inv6._yscale = 400;
	inv6._y = 100; inv6._x = 200;
	inv6.onRollOver = function() {};

	// Point inside the inner square hits the shape !
	check( inv6.hitTest(200 + (15*4), 100 + (15*4), true) ); 
	// As points on the outer borders
	check( inv6.hitTest(200 + (11*4), 100 + (11*4), true) );  // Upper-Left
	check( inv6.hitTest(200 + (11*4), 100 + (14*4), true) );  // Center-Left
	check( inv6.hitTest(200 + (11*4), 100 + (19*4), true) );  // Lower-Left
	check( inv6.hitTest(200 + (14*4), 100 + (19*4), true) );  // Lower-Center
	check( inv6.hitTest(200 + (19*4), 100 + (19*4), true) );  // Lower-Right
	check( inv6.hitTest(200 + (19*4), 100 + (14*4), true) );  // Center-Right
	check( inv6.hitTest(200 + (19*4), 100 + (11*4), true) );  // Upper-Right
	check( inv6.hitTest(200 + (14*4), 100 + (11*4), true) );  // Upper-Center

	// Nested squares, calling beginFill again after the move)
	// Outer square counterclockwise order, inner in clockwise order
	createEmptyMovieClip("inv7", 7);
	with (inv7)
	{
		lineStyle(0, 0); // hairline
		beginFill(0x00FF00);

		moveTo(10, 10);
		lineTo(10, 20);
		lineTo(20, 20);
		lineTo(20, 10);
		lineTo(10, 10);

		moveTo(12, 12);

		// this forces endFill call, which triggers finalization of previous path (I think)
		beginFill(0x00FF00);

		lineTo(18, 12);
		lineTo(18, 18);
		lineTo(12, 18);
		lineTo(12, 12);
	}
	inv7._xscale = inv7._yscale = 400;
	inv7._y = 100; inv7._x = 250;
	inv7.onRollOver = function() {};

	// Point inside the inner square hits the shape !
	check( inv7.hitTest(250 + (15*4), 100 + (15*4), true) ); 
	// As points on the outer borders
	check( inv7.hitTest(250 + (11*4), 100 + (11*4), true) );  // Upper-Left
	check( inv7.hitTest(250 + (11*4), 100 + (14*4), true) );  // Center-Left
	check( inv7.hitTest(250 + (11*4), 100 + (19*4), true) );  // Lower-Left
	check( inv7.hitTest(250 + (14*4), 100 + (19*4), true) );  // Lower-Center
	check( inv7.hitTest(250 + (19*4), 100 + (19*4), true) );  // Lower-Right
	check( inv7.hitTest(250 + (19*4), 100 + (14*4), true) );  // Center-Right
	check( inv7.hitTest(250 + (19*4), 100 + (11*4), true) );  // Upper-Right
	check( inv7.hitTest(250 + (14*4), 100 + (11*4), true) );  // Upper-Center

	//------------------------------------------------
	//
	//  Single path with complex crossing
	//
	//
	//  10     5----4,0----------1
	//         |#####|###########|
	//         |#####|###########|
	//  20     6-----+----7######|
	//               |    |######|
	//               |    |######|
	//  30           9----8######|
	//               |###########|
	//  40           3-----------2
	//
	//         10   20    30     40
	//
	//------------------------------------------------
	createEmptyMovieClip("inv8", 8);
	with (inv8)
	{
		beginFill(0xFF0000);
		moveTo(20, 10);  // 0
		lineTo(40, 10);  // 1
		lineTo(40, 40);  // 2
		lineTo(20, 40);  // 3
		lineTo(20, 10);  // 4

		lineTo(10, 10);   // 5
		lineTo(10, 20);   // 6
		lineTo(30, 20);  // 7
		lineTo(30, 30);  // 8
		lineTo(20, 30);  // 9

		// implicit close-up to 0 (aka 4, aka 20,10)
	}

	inv8._xscale = inv8._yscale = 200;
	inv8._x = 200;
	inv8.onRollOver = function() {};

	check(  inv8.hitTest(200 + (15*2), 0 + (15*2), true) );  // Upper-Left
	check(  inv8.hitTest(200 + (25*2), 0 + (15*2), true) );  // Upper-Center
	check(  inv8.hitTest(200 + (35*2), 0 + (15*2), true) );  // Upper-Right

	check( !inv8.hitTest(200 + (15*2), 0 + (25*2), true) );  // Center-Left
	check( !inv8.hitTest(200 + (25*2), 0 + (25*2), true) );  // Center-Center
	check(  inv8.hitTest(200 + (35*2), 0 + (25*2), true) );  // Center-Right

	check( !inv8.hitTest(200 + (15*2), 0 + (35*2), true) );  // Lower-Left
	check(  inv8.hitTest(200 + (25*2), 0 + (35*2), true) );  // Lower-Center
	check(  inv8.hitTest(200 + (35*2), 0 + (35*2), true) );  // Lower-Right

	check( !inv8.hitTest(200 + (20*2), 0 + (25*2), true) );  // On the 0-9 stroke, out of fill


	_visible = false;
}

createEmptyMovieClip("line", 110);
with(line) {

    col = 0xff0000;
    colc = 100;
    x = 20;
    y = 20;
    xr = 100;
    yr = 100;
    n = 10;
    alpha = 100;
    
    // =======================
    //   Shape 1
    // =======================

    beginFill(col,colc);
    lineStyle(n,0x0000ff,alpha);

    moveTo(x,y);
    lineTo(x+xr,y);
    lineTo((x+xr),(y+yr));

    // In Gnash, next line forces a new path,
    // and closeup of the former (shouldn't)
    // 
    lineStyle(n,0x00ff00,alpha);

    lineTo(x,(y+yr));
    lineTo(x,y);

    endFill();

    // =======================
    //   Shape 2
    // =======================

    x += 200;

    beginFill(col,colc);
    lineStyle(n,1,alpha);

    moveTo(x,y);
    lineTo(x+xr,y);

    lineStyle(n,0x0000ff,alpha);

    lineTo((x+xr),(y+yr));

    lineStyle(n,0x00ff00,alpha);

    lineTo(x,(y+yr));
    lineTo(x,y);

    endFill(); 

    // =======================
    //   Shape 3
    // =======================

    x += 200;

    createEmptyMovieClip("d", 500);
    with (d)
    {
        lineStyle(10, 10, 100);
        moveTo(x, y);
        lineTo(x, y + yr);
        lineTo(x + xr, y + yr);
        endFill(); // does NOT force closeup
    }

    // =======================
    //   Shape 4
    // =======================

    x = 20;
    y += 150;

    createEmptyMovieClip("e", 501);
    with (e)
    {
        lineStyle(10, 10, 100);
        beginFill(); // does NOT start a fill
        moveTo(x, y);
        lineTo(x, y + yr);
        lineTo(x + xr, y + yr);
        endFill(); // does NOT force closeup
    }

    // The shape is not closed to form a triangle,
    // so the only way to hit it is to hit the line.
    check(!e.hitTest(x + 30, y + 30, true));

    // Top right 'corner'
    check(e.hitTest(x, y, true));

    // =======================
    //   Shape 5
    // =======================

    x += 200;

    createEmptyMovieClip("f", 502);
    with (f)
    {
        lineStyle(10, 10, 100);
        beginFill(0); // does start a fill
        moveTo(x, y);
        lineTo(x, y + yr);
        lineTo(x + xr, y + yr);
        endFill(); // DOES force closeup
    }
    
    // Somewhere within the closed triangle.
    check(f.hitTest(x + 30, y + 30, true));

    // =======================
    //   Shape 6
    // =======================
    
    x += 200;

    createEmptyMovieClip("g", 503);
    with (g)
    {
        lineStyle(10, 0xFFFF00, 100);
        beginFill(0x00ff00); // does start a fill
        moveTo(x, y);
        lineTo(x, y + yr);
        lineTo(x + xr, y + yr);
        beginFill(0x0000ff); // closes previous fill, opens a new one
        lineTo(x + xr, y);
        lineTo(x + xr, y + yr);
        endFill(); // DOES force closeup
    }

    // Bottom left
    check(g.hitTest(x + 20, y + 80, true));

    // Top right
    // COMPATIBLITY NOTE:
    // 9,0,115,0 for linux sometimes finds the point to hit
    // the shape and sometimes it doesn't.
    // 10,0,12,10 never succeeds the hit test.
    //
    // The point is clearly within a fill so we'll take an hit test
    // success as expected.
    //
    check(g.hitTest(x + 80, y + 20, true));

    _visible = false;
}
line.onRollOver = function() {};

// Check the the top right diagonal half of Shape 2.
xcheck(line.hitTest(270, 40, true));

// Check the bottom left diagonal half of Shape 2.
check(line.hitTest(230, 80, true));


// Page 4 ( page[3] )

draw100x100Box = function(x, y, mc) {
    s = 90;
    with (mc) {
        moveTo(x, y);
        lineTo(x + s, y);
        lineTo(x + s, y + s);
        lineTo(x, y + s);
        lineTo(x, y);
        endFill();
    };
};
    

createEmptyMovieClip("grad", 150);

// Test gradients.
// The beginGradientFill function works with fake Matrices, but there is no
// point making more work for ourselves as that testing is already done for
// the Matrix class.
// Only the "box" matrixType gets special handling.

with(grad) {

    // Linear gradients
    fillType = "linear";

    x = 0;
    y = 0;

    // shape 1
    colors = [0x0000ff, 0xffffff];
    alphas = [100, 100];
    ratios = [0, 0xff];
    matrix = new flash.geom.Matrix();
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
    
    // shape 2
    x += 100;
    colors = [0x0000ff, 0xffffff];
    alphas = [100, 100];
    ratios = [0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
    
    // shape 3
    x += 100;
    colors = [0x0000ff, 0xffffff, 0xff00ff];
    alphas = [100, 100, 100];
    ratios = [0, 0xff / 2, 0xff];
    matrix.createGradientBox(90, 90, Math.PI / 4, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    // shape 4
    x += 100;
    colors = [0x0000ff, 0xffffff, 0xff00ff];
    alphas = [100, 100, 100];
    ratios = [0, 0xff / 2, 0xff];
    matrix.createGradientBox(180, 180, Math.PI / 4, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    // shape 5
    x += 100;
    colors = [0x0000ff, 0xffffff, 0x00ff00];
    alphas = [100, 100, 100];
    ratios = [0, 0xff / 2, 0xff];
    matrix.createGradientBox(180, 180, Math.PI / 4 * 3, x - 90, y - 90);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    // shape 6
    x += 100;
    colors = [0x0000ff, 0xffffff, 0x00ff00, 0xff00ff, 0x00ffff, 0xffff00 ];
    alphas = [100, 100, 100, 50, 25, 100];
    ratios = [0, 0xff / 5, 0xff / 5 * 2, 0xff / 5 * 3, 0xff / 5 * 4, 0xff];
    matrix.createGradientBox(90, 90, Math.PI / 2, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
 
    // Radial gradients
    fillType = "radial";

    x = 0;
    y += 100;

    // shape 7
    colors = [0x0000ff, 0xffffff];
    alphas = [100, 100];
    ratios = [0, 0xff];
    matrix = new flash.geom.Matrix();
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
    
    // shape 8
    x += 100;
    colors = [0x0000ff, 0xffffff];
    alphas = [100, 100];
    ratios = [0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
    
    // shape 9
    x += 100;
    colors = [0x0000ff, 0xffffff, 0xff00ff];
    alphas = [100, 100, 100];
    ratios = [0, 0xff / 2, 0xff];
    matrix.createGradientBox(90, 90, Math.PI / 4, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    // shape 10
    x += 100;
    colors = [0x0000ff, 0xffffff, 0xff00ff];
    alphas = [100, 100, 100];
    ratios = [0, 0xff / 2, 0xff];
    matrix.createGradientBox(180, 180, Math.PI / 4, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    // shape 11
    x += 100;
    colors = [0x0000ff, 0xffffff, 0x00ff00];
    alphas = [100, 100, 100];
    ratios = [0, 0xff / 2, 0xff];
    matrix.createGradientBox(180, 180, Math.PI / 4 * 3, x - 90, y - 90);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    // shape 12
    x += 100;
    colors = [0x0000ff, 0xffffff, 0x00ff00, 0xff00ff, 0x00ffff, 0xffff00 ];
    alphas = [100, 100, 100, 50, 25, 100];
    ratios = [0, 0xff / 5, 0xff / 5 * 2, 0xff / 5 * 3, 0xff / 5 * 4, 0xff];
    matrix.createGradientBox(90, 90, Math.PI / 2, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    y += 100;
    x = 0;

    // Shape 13

    // Check that ratios are adjusted if they do not get successively larger.
    fillType = "linear";
    colors = [0x0000ff, 0xffffff, 0x00ff00];
    alphas = [100, 100, 100];
    ratios = [0, 0x10, 0x05];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
   

    // Shape 14
    x += 100;

    // Test a linear gradient with one stop
    fillType = "linear";
    colors = [0xff0000];
    alphas = [100];
    ratios = [0];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
   

    // Shape 15
    x += 100;

    // Test a linear gradient with one stop
    fillType = "radial";
    colors = [0x00ff00];
    alphas = [100];
    ratios = [0];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);
    
    // Shape 16
    x += 100;

    // Bad fill style results in no fill.
    fillType = "rodial";
    colors = [0x0000ff, 0xffffff, 0x00ff00];
    alphas = [100, 100, 100];
    ratios = [0, 0x10, 0x05];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix);
    draw100x100Box(x, y, grad);

    // Shape 17
    x += 100;

    // Extra arguments result in no fill for SWF6
    fillType = "radial";
    colors = [0x0000ff, 0xffffff, 0x00ff00];
    alphas = [100, 100, 100];
    ratios = [0, 0x10, 0x05];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad");
    draw100x100Box(x, y, grad);

    x = 0;
    y += 100;

    // Shape 18

    // 4 arguments result in no fill for SWF6
    fillType = "radial";
    colors = [0x0000ff, 0xffffff, 0x00ff00];
    alphas = [100, 100, 100];
    ratios = [0, 0x10, 0x05];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios);
    draw100x100Box(x, y, grad);

    _visible = false;

};
grad.onRollOver = function() {};

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

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
		
// don't move the controls for now...
#if 0
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

		var ch = _root.page[_root.visibleIndex];

		// Bounding box check
		if ( hitTest(ch) ) {
			hd.shapeshape._xscale=150;
			hd.shapeshape._yscale=150;
		} else {
			hd.shapeshape._xscale=100;
			hd.shapeshape._yscale=100;
		}

		// Bounding box check with circle center
		if ( ch.hitTest(_x, _y) ) {
			hd.bboxpoint._xscale=150;
			hd.bboxpoint._yscale=150;
		} else {
			hd.bboxpoint._xscale=100;
			hd.bboxpoint._yscale=100;
		}

		// Shape check with circle center
		if ( ch.hitTest(_x, _y, true) ) {
			hd.shapepoint._xscale=150;
			hd.shapepoint._yscale=150;
		} else {
			hd.shapepoint._xscale=100;
			hd.shapepoint._yscale=100;
		}
	};
}


isMask = false;
onMouseDown = function()
{
	var ch = _root.page[_root.visibleIndex];

	if ( isMask )
	{
		ch.setMask(); // no effect !
		ch.setMask(true); // no effect !
		trace("Disabling cursor mask");
		ch.setMask(undefined); // works
		//a.setMask(null); // also work
	}
	else
	{
		trace("Enabling cursor mask");
		ch.setMask(cursor);
	}
	isMask = !isMask;
};



visibleIndex = 0;
page = new Array;
page[0] = a;
page[1] = inv;
page[2] = line;
page[3] = grad;

onKeyDown = function()
{
	var ascii = Key.getAscii();
	if ( ascii >= 48 && ascii <= 57 ) // 0..9 - activate corresponding drawing 
	{
		with (page[visibleIndex])
		{
			_visible = false;
			setMask(null);
		}

		visibleIndex = parseInt(ascii)-49;
		trace("Key "+visibleIndex+" hit");

		with (page[visibleIndex])
		{
			_visible = true;
		}
	}
	else if ( ascii == 104 ) // 'h' - toggle visibility
	{
		page[visibleIndex]._visible = ! page[visibleIndex]._visible;
	}
	else if ( ascii == 45 ) // '-' - decrease alpha
	{
		var newAlpha = page[visibleIndex]._alpha - 25;
		if ( newAlpha < 0 ) newAlpha = 0;
		page[visibleIndex]._alpha = newAlpha;
	}
	else if ( ascii == 43 ) // '+' - increase alpha
	{
		var newAlpha = page[visibleIndex]._alpha + 25;
		if ( newAlpha > 100 ) newAlpha = 100;
		page[visibleIndex]._alpha = newAlpha;
	}
	else if ( ascii == 100 ) // 'd' - start/stop dragging something :)
	{
		if ( ! hitdetector.dragging ) {
			hitdetector.dragging = true;
			hitdetector.startDrag();
			// hitdetector.startDrag(true); // lock centered
			//hitdetector.startDrag(true, 100, 100, 200, 200); // lock centered, box-constrained
		} else {
			hitdetector.dragging = flase;
			hitdetector.stopDrag();
		}
	}

};
Key.addListener(this);

stop();


