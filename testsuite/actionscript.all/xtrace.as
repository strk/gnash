createTextField("out",300000,0,0,600,800);

// FIXME: _global object isn't recognized
//_global.xtrace = function (msg)

xtrace = function (msg) 
{
	_level0.out.text += msg+"\n";
};


xtrace("Xtrace working");
