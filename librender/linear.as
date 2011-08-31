// "linear" or "radial"
fillType = "linear";
// The maximum possible number of colors you can use is 15.
colors = [0xFF0000, 0x0000FF];
// the transparency of colors you defined in the previous
// parameter. The possible values range from 0 (completely
// transparent) to 100 (completely opaque). If you specify a value
// less then 0, Flash will use 0. If you enter a value greater than
// 100, Flash will use 100. It makes sense, because you can’t have
// something more transparent than alpha put at 0, beacuse a color or
// a drawing with alpha 0 is invisible.
alphas = [100, 100];
// The ratio defines where in your gradient the color it is associated
// with is at its 100% value. Or, the place where that color is pure
// and hasn’t started mixing with the other color(s) yet.
ratios = [0, 255];

// matrixType This value should always be "box".
// x The x coordinate of the bottom-left corner of the gradient.
// y The y coordinate of the bottom-left corner of the gradient.
// width The width of the gradient in pixels.
// height The height of the gradient in pixels.
// r The rotation of the gradient in radians (not degrees).
//
// For box type, the x should be the left margin, the y can be
// anything. The w is calculated by right margin-left margin, the h
// can be anything except 0. 
// matrix = {matrixType:"box", x:150, y:200, w:350, h:200, r:0/180*Math.PI};
matrix = {matrixType:"box", x:100, y:100, w:400, h:300, r:0};
_root.lineStyle(1, 0x000000, 100);

_root.beginGradientFill(fillType, colors, alphas, ratios, matrix);

//             X (200)                  W (150)
//             |                        |
// (x0, y0) 200,200------------------500,200 (x1, y0)
//             |                        |
//             |                        |---- Y (300)
//             |                        |
// (x0, y1) 200,400------------------500,400 (x1, y1)_
//

// This is the dimensions of the big black box that is supposed to have
// a linear gradient in it.
x0 = 100; // 200 pixels = 4000 twips
y0 = 100; // 200 pixels = 4000 twips
x1 = 500; // 500 pixels = 10000 twips
y1 = 500; // 400 pixels = 8000 twips
_root.moveTo(x0, y0);
_root.lineTo(x1, y0);
_root.lineTo(x1, y1);
_root.lineTo(x0, y1);
_root.lineTo(x0, y0);

_root.endFill();

 
