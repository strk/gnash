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

with (a)
{
	clear();

	// The thick red line
	lineStyle(20, 0xFF0000, 100);
	moveTo(100, 100);
	lineTo(200, 200);

	// The hairlined horizontal black line
	lineStyle(0, 0x000000, 100);
	moveTo(220, 180);
	lineTo(280, 180);

	// The violet line
	moveTo(100, 200);
	lineStyle(5, 0xFF00FF, 50);
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

	// clockwise
	moveTo(200, 100);
	lineStyle(1, 0x00FF00);
	beginFill(0x00FFFF, 100);
	lineTo(200, 120);
	lineTo(180, 120);
	lineTo(180, 100);
	lineTo(200, 100);
	endFill();

	// counter-clockwise
	moveTo(230, 100);
	lineStyle(1, 0x00FFFF);
	beginFill(0x00FF00, 50);
	lineTo(210, 100);
	lineTo(210, 120);
	lineTo(230, 120);
	lineTo(230, 100);
	endFill();

	// clockwise, move after beginFill
	lineStyle(1, 0x00FF00);
	beginFill(0xFF00FF, 100);
	moveTo(260, 100);
	lineTo(260, 120);
	lineTo(240, 120);
	lineTo(240, 100);
	lineTo(260, 100);
	endFill();
}

// Make the MovieClip "active" (grabbing mouse events)
// This allows testing of fill styles and "thick" lines
a.onRollOver = function() {};

