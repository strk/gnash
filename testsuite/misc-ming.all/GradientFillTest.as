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
 
// This tests the gradients found in DrawingApiTest for version 8, where
// some extra arguments were added.

// Make Matrix visible for easier gradient tests.
ASSetPropFlags(_global, "flash", 0, 5248);

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

    // Shape 15
    x += 100;

    // Test a radial gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad");
    draw100x100Box(x, y, grad);
    
    // Shape 16
    x += 100;

    // Test a radial gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "reflect");
    draw100x100Box(x, y, grad);

    // Shape 17
    x += 100;

    // Test a radial gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "repeat");
    draw100x100Box(x, y, grad);
    
    x = 0;
    y += 100;
    
    // Shape 18

    // Test a radial gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "");
    draw100x100Box(x, y, grad);
    
    // Shape 19
    x += 100;

    // Test a radial gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad",
            "RGB");
    draw100x100Box(x, y, grad);


    // Shape 20
    x += 100;
    
    // Test a radial gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad",
            "linearRGB");
    draw100x100Box(x, y, grad);

    // Shape 21
    x += 100;
    
    // Test a radial gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad",
            "");
    draw100x100Box(x, y, grad);

    // Shape 22
    x += 100;
    
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad",
            "rgb", 3.5);
    draw100x100Box(x, y, grad);

    // Shape 23
    x += 100;
    
    fillType = "linear";
    colors = [0xff0000, 0x00ff00];
    alphas = [100, 100];
    ratios = [0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad",
            "linearRGB", 3.5);
    draw100x100Box(x, y, grad);

    // Shape 24
    x = 0;
    y += 100;
    
    fillType = "linear";
    colors = [0xff0000, 0x00ff00];
    alphas = [100, 0];
    ratios = [0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad",
            "linearRGB", 3.5);
    draw100x100Box(x, y, grad);

};
grad.onRollOver = function() {};

testbmp = new flash.display.BitmapData(1000, 1000, false);
testbmp.draw(grad);

searchArray = function(ar, el) {
    for (var i in ar) {
        if (ar[i] == el) return true;
    };
    return false;
};

nearColor = function(a, b, exfail) {
   tolerance = 32;
   ra = (a & 0xff0000) >> 16;
   ba = (a & 0xff00) >> 8;
   ga = (a & 0xff);
   rb = (b & 0xff0000) >> 16;
   bb = (b & 0xff00) >> 8;
   gb = (b & 0xff);
   dist = Math.sqrt(Math.pow(ra - rb, 2) + Math.pow(ba - bb, 2) + Math.pow(ga - gb, 2));
   if (dist > tolerance) {
       str = "Expected: 0x" + b.toString(16) + ", got: 0x" + a.toString(16);
       str += " (Allowed tolerance: " + tolerance + ", distance: " + dist + ")";
       if (exfail) xfail_check(str);
       else fail_check(str);
       return;
   }
   str = "0x" + b.toString(16) + " is near to 0x" + a.toString(16);
   if (exfail) xpass_check(str);
   else pass_check(str);
};

// Array is:
// 0: top left
// 1: top centre
// 2: top right
// 3: centre left
// 4: centre centre
// 5: centre right
// 6: bottom left
// 7: bottom centre
// 8: bottom right

// Passes is an optional array of the index of the tests that
// fail.
checkSquare = function(x, y, ar, bmp, fails) {
    trace("Checking square at " + x + "x" + y);
    size = 90;
    low = 4;
    high = size - low;
    mid = size / 2;
#if 1
    t = [
        "0x" + testbmp.getPixel(x + low, y + low).toString(16),
        "0x" + testbmp.getPixel(x + low, y + mid).toString(16),
        "0x" + testbmp.getPixel(x + low, y + high).toString(16),
        "0x" + testbmp.getPixel(x + mid, y + low).toString(16),
        "0x" + testbmp.getPixel(x + mid, y + mid).toString(16),
        "0x" + testbmp.getPixel(x + mid, y + high).toString(16),
        "0x" + testbmp.getPixel(x + high, y + low).toString(16),
        "0x" + testbmp.getPixel(x + high, y + mid).toString(16),
        "0x" + testbmp.getPixel(x + high, y + high).toString(16)
    ];
    trace(t);
#endif 
    nearColor(testbmp.getPixel(x + low, y + low), ar[0], searchArray(fails, 0));
    nearColor(testbmp.getPixel(x + low, y + mid), ar[1], searchArray(fails, 1));
    nearColor(testbmp.getPixel(x + low, y + high), ar[2], searchArray(fails, 2));
    nearColor(testbmp.getPixel(x + mid, y + low), ar[3], searchArray(fails, 3));
    nearColor(testbmp.getPixel(x + mid, y + mid), ar[4], searchArray(fails, 4));
    nearColor(testbmp.getPixel(x + mid, y + high), ar[5], searchArray(fails, 5));
    nearColor(testbmp.getPixel(x + high, y + low), ar[6], searchArray(fails, 6));
    nearColor(testbmp.getPixel(x + high, y + mid), ar[7], searchArray(fails, 7));
    nearColor(testbmp.getPixel(x + high, y + high), ar[8], searchArray(fails, 8));
};

checkSquare(0, 0, [0x8080ff,0x8080ff,0x8080ff,0x8787ff,0x8787ff,0x8787ff,0x8d8dff,0x8d8dff,0x8d8dff], bmp);
checkSquare(100, 0, [0xb0bff,0xb0bff,0xb0bff,0x7f7fff,0x7f7fff,0x7f7fff,0xf4f4ff,0xf4f4ff,0xf4f4ff], bmp);
checkSquare(200, 0, [0xff,0x5a5aff,0xfffdff,0x5a5aff,0xffffff,0xff59ff,0xffffff,0xff59ff,0xff00ff], bmp);
checkSquare(300, 0, [0xff,0xff,0x4a4aff,0xff,0x4a4aff,0x9c9cff,0x4a4aff,0x9c9cff,0xeeeeff], bmp);
checkSquare(400, 0, [0xffffff,0xabffab,0x59ff59,0xacacff,0xffffff,0xabffab,0x5a5aff,0xacacff,0xffffff], bmp);
checkSquare(500, 0, [0x3737ff,0xa29ea2,0xd1ff57,0x3737ff,0xa29ea2,0xd1ff57,0x3737ff,0xa29ea2,0xd1ff57], bmp);

checkSquare(0, 100, [0x2020ff,0x2d2dff,0x3a3aff,0x2323ff,0x2f2fff,0x3b3bff,0x2a2aff,0x3434ff,0x4040ff], bmp);
checkSquare(100, 100, [0xffffff,0xe9e9ff,0xffffff,0xe9e9ff,0xff,0xe9e9ff,0xffffff,0xe9e9ff,0xffffff], bmp);
checkSquare(200, 100, [0xff00ff,0xff2bff,0xff00ff,0xff2bff,0xff,0xff2bff,0xff00ff,0xff2bff,0xff00ff], bmp);
checkSquare(300, 100, [0xff00ff,0xff00ff,0xff15ff,0xff00ff,0xff93ff,0xfffdff,0xff15ff,0xfffdff,0x2020ff], bmp);
checkSquare(400, 100, [0x2020ff,0xfdfffd,0x15ff15,0xfdfffd,0x93ff93,0xff00,0x15ff15,0xff00,0xff00], bmp);
checkSquare(500, 100, [0xffff00,0xb4ff9d,0xffff00,0xb4ff9d,0xff,0xb4ff9d,0xffff00,0xb4ff9d,0xffff00], bmp);

checkSquare(0, 200, [0xafafff,0xafafff,0xafafff,0xff00,0xff00,0xff00,0xff00,0xff00,0xff00], bmp);
checkSquare(100, 200, [0xff0000,0xff0000,0xff0000,0xff0000,0xff0000,0xff0000,0xff0000,0xff0000,0xff0000], bmp);
checkSquare(200, 200, [0xff00,0xff00,0xff00,0xff00,0xff00,0xff00,0xff00,0xff00,0xff00], bmp);
checkSquare(300, 200, [0xffff,0xc3ff,0xffff,0xc3ff,0xffff00,0xc3ff,0xffff,0xc3ff,0xffff], bmp);
checkSquare(400, 200, [0x55ff,0xa9ff,0x43ff,0xb6ff,0xeded11,0xd1ff,0x30ff,0xdeff,0x1dff], bmp);
checkSquare(500, 200, [0x9d9d61,0xa3ff,0x91916d,0xb6ff,0xeaea14,0xd1ff,0x82827c,0xe4ff,0x757589], bmp, [7]);

checkSquare(0, 300, [0xffff,0xc3ff,0xffff,0xc3ff,0xffff00,0xc3ff,0xffff,0xc3ff,0xffff], bmp);
checkSquare(100, 300, [0xffff,0xc3ff,0xffff,0xc3ff,0xffff00,0xc3ff,0xffff,0xc3ff,0xffff], bmp);
checkSquare(200, 300, [0xfefe,0xe2fe,0xfefe,0xe2fe,0xfefe00,0xe2fe,0xfefe,0xe2fe,0xfefe], bmp, [4]);
checkSquare(300, 300, [0xffff,0xc3ff,0xffff,0xc3ff,0xffff00,0xc3ff,0xffff,0xc3ff,0xffff], bmp);
checkSquare(400, 300, [0xffff,0xe1ff,0xffff,0xc6ff,0x3636c8,0xc6ff,0xffff,0xf2f20c,0xffff], bmp);
checkSquare(500, 300, [0xfa3a00,0xfa3a00,0xfa3a00,0xbbbb00,0xbbbb00,0xbbbb00,0x3afa00,0x3afa00,0x3afa00], bmp);

checkSquare(0, 400, [0xfa420b,0xfa420b,0xfa420b,0xdddd7f,0xdddd7f,0xdddd7f,0xf6fff4,0xf6fff4,0xf6fff4], bmp);
totals();

stop();


