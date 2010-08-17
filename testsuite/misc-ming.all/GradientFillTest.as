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
    
    // Test a focal gradient with SWF8 args
    fillType = "radial";
    colors = [0xffff00, 0x0000ff, 0x00ffff];
    alphas = [100, 100, 100];
    ratios = [0, 0xa0, 0xff];
    matrix.createGradientBox(90, 90, 0, x, y);
    beginGradientFill(fillType, colors, alphas, ratios, matrix, "pad",
            "RGB", 3.5);
    draw100x100Box(x, y, grad);

};
grad.onRollOver = function() {};

stop();


