// makeswf -o StageConfigTest.swf -s 512x512 StageConfigTest.as
//
// Test for stage configuration. Meant for manual testing, will need
// to be turned into a self-contained test (volunteers?)
//

createEmptyMovieClip("s", 1);
with (s)
{
	x=2;

	lineStyle(0, 0);
	moveTo(0*x, 0*x);
	lineTo(256*x,256*x);
	lineTo(256*x,0*x);
	lineTo(0*x, 256*x);
	lineTo(0*x, 0*x);
};

s.createEmptyMovieClip("h", 1);
with(s.h)
{
	lineStyle(0, 0);
	beginFill(0xffffff, 256);
	moveTo(256-10, 256-10);
	lineTo(256+10, 256-10);
	lineTo(256+10, 256+10);
	lineTo(256-10, 256+10);
	moveTo(256-10, 256-10);
}
s.h.onRollOver = function() {};

createTextField("tf", 99, 256-100, 0, 200, 50);
tf.border = false;
tf.autoSize = "center";

createTextField("atf", 98, 10, 130, 100, 50);
atf.text = "Align: " + Stage.align;

createTextField("stf", 97, 10, 150, 200, 50);
stf.text = "Scale mode: " + Stage.scaleMode;

createTextField("ss", 96, 10, 170, 200, 50);
function updateStageSizeReport()
{
	ss.text = "Stage size: "+Stage.width+"x"+Stage.height;
}
updateStageSizeReport();

onMouseMove = function()
{
	s = "Mouse coords:"+_xmouse+"x"+_ymouse;
	updateStageSizeReport();
	tf.text = s;
};

scaleModeValues = ['showAll','noScale','exactFit','noBorder'];
scaleModeValue=0;
onKeyDown = function()
{
        var ascii = Key.getAscii();
	var char = String.fromCharCode(ascii);

        if ( char == 's' ) 
	{
		scaleModeValue=(scaleModeValue+1)%scaleModeValues.length;
		Stage.scaleMode = scaleModeValues[scaleModeValue];
		trace("scaleMode:"+Stage.scaleMode);
	}
	else if (char == 'c')
	{
		Stage.align = "";
		r=b=t=l=false;
	}
	else
	{
		if (char == "r") { r = !r; }
		if (char == "b") { b = !b; }
		if (char == "t") { t = !t; }
		if (char == "l") { l = !l; }
 
		al = r ? "r" : "";
		al += b ? "b" : "";
		al += l ? "l" : "";
		al += t ? "t" : "";    
	 
		trace("align = "+al);
		Stage.align = al;
	}
	
	atf.text="Align: " + Stage.align;
	stf.text = "Scale mode: " + Stage.scaleMode;
	
	// update mouse too
	s = _xmouse+"x"+_ymouse;
	tf.text = s;

	updateStageSizeReport();
	
};
Key.addListener(this);

onResize = function()
{
        trace("Resize event received, args to handler: "+arguments.length+" Stage.width="+Stage.width+", Stage.height="+Stage.height);
	updateStageSizeReport();
};
Stage.addListener(this);
