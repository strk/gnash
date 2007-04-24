//
// Some tests for the Drawing API
// Build with:
//	makeswf -o DrawingApi.swf DrawingApi.as
// Run with:
//	firefox DrawingApi.swf
// Or:
//	gnash DrawingApi.swf
//

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

	// The thick red line
	lineStyle(thick, red, 100);
	moveTo(100, 100);
	lineTo(200, 200);

	// The hairlined horizontal black line
	lineStyle(0, 0x000000, 100);
	moveTo(220, 180);
	lineTo(280, 180);

	// The violet line
	moveTo(100, 200);
	lineStyle(5, 0xFF00FF, halftransparent);
	lineTo(200, 250);

	// The yellow line
	lineStyle(10, 0xFFFF00, 100);
	lineTo(400, 200);

	// The green curve
	lineStyle(8, 0x00FF00, 100);
	curveTo(400, 120, 300, 100);

	// Transparent line
	lineStyle();
	lineTo(80, 100);

	// The black thick vertical line
	lineStyle(20);
	lineTo(80, 150);

	// The ugly blue-fill red-stroke thingy
	moveTo(80, 180);
	lineStyle(2, 0xFF0000);
	beginFill(0x0000FF, 100);
	lineTo(50, 180);
	curveTo(20, 200, 50, 250);
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
}

// Make the MovieClip "active" (grabbing mouse events)
// This allows testing of fill styles and "thick" lines
a.onRollOver = function() {};

