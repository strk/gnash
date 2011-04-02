fillType = "linear";
colors = [0xFF0000, 0x0000FF];
alphas = [100, 100];
ratios = [0, 255];

// This appears to be a "box" within the gradient, that effects how
// wide the color stripes are.
matrix = {matrixType:"box", x:150, y:200, w:500, h:200, r:0/180*Math.PI};
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
x0 = 200; // 200 pixels = 4000 twips
y0 = 200; // 200 pixels = 4000 twips
x1 = 500; // 500 pixels = 10000 twips
y1 = 400; // 400 pixels = 8000 twips
_root.moveTo(x0, y0);
_root.lineTo(x1, y0);
_root.lineTo(x1, y1);
_root.lineTo(x0, y1);
_root.lineTo(x0, y0);

_root.endFill();

 
