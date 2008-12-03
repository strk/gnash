
function Checkbox(where, label) {
	if ( ! where.hasOwnProperty('nextHighestDepth') ) where.nextHighestDepth=1;
	var d = where.nextHighestDepth++;
	var nam = 'cb'+d;
	this.clip = where.createEmptyMovieClip(nam, d);
	this.size = 10;

	this.clip.createEmptyMovieClip('box', 1);
	with (this.clip.box)
	{
		lineStyle(1, 100, 100);
		beginFill(0, 0);
		moveTo(0, 0);
		lineTo(this.size, 0);
		lineTo(this.size, this.size);
		lineTo(0, this.size);
		lineTo(0, 0);
		endFill();
	}
	this.clip.box.cb = this;
	this.clip.box.onRelease = function()
	{
		if ( this.cb.checked() ) this.cb.uncheck();
		else this.cb.check();
	};
	this.clip.createEmptyMovieClip('check', 2);
	with (this.clip.check)
	{
		lineStyle(1, 100, 100);
		moveTo(0, 0);
		lineTo(this.size, this.size);
		moveTo(this.size, 0);
		lineTo(0, this.size);
	}
	this.clip.check._visible=false;

	this.clip.createTextField('label', 3, this.size+this.size, 0, 0, 0);
	this.clip.label.autoSize = true;
	this.clip.label.text = label;
}

Checkbox.prototype.check = function()
{
	trace("Making check visible");
	this.clip.check._visible=true;
};

Checkbox.prototype.uncheck = function()
{
	trace("Making check invisible");
	this.clip.check._visible=false;
};

Checkbox.prototype.checked = function()
{
	trace("Checkbox checked? "+this.clip.check._visible);
	return this.clip.check._visible;
};

Checkbox.prototype.valueOf = function()
{
	return this.checked();
};

Checkbox.prototype.moveTo = function(x, y)
{
	this.clip._x = x;
	this.clip._y = y;
};

// ------------------------------------------------------------------

function Button(where, label, cb)
{
	if ( ! where.hasOwnProperty('nextHighestDepth') ) where.nextHighestDepth=1;
	var d = where.nextHighestDepth++;
	var nam = 'bt'+d;
	this.clip = where.createEmptyMovieClip(nam, d);
	this.clip.createEmptyMovieClip('eh', 1); // event handler
	this.clip.onRelease = cb;

	this.clip.createTextField('label', 3, this.size+this.size, 0, 0, 0);
	this.clip.label.autoSize = true;
	this.clip.label.text = label;

	this.w = this.clip.label._width;
	this.h = this.clip.label._height;
	with (this.clip.eh)
	{
		lineStyle(1, 100, 100);
		beginFill(0, 0);
		moveTo(0, 0);
		lineTo(this.w, 0);
		lineTo(this.w, this.h);
		lineTo(0, this.h);
		lineTo(0, 0);
		endFill();
	}
}

Button.prototype.moveTo = function(x, y)
{
	this.clip._x = x;
	this.clip._y = y;
};

//-------------------

function Input(where, label)
{
	if ( ! where.hasOwnProperty('nextHighestDepth') ) where.nextHighestDepth=1;
	var d = where.nextHighestDepth++;
	var nam = 'bt'+d;
	this.clip = where.createEmptyMovieClip(nam, d);

	this.clip.createTextField('label', 3, 0, 0, 0, 0, 0);
	this.clip.label.autoSize = true;
	this.clip.label.text = label;

	this.clip.createTextField('inp', 4, 0, 0, 100, 20); // TODO: take as params
	this.clip.inp.autoSize = false;
	this.clip.inp.type = 'input';
	this.clip.inp.border = true;
	this.clip.inp.text = 'your input here';
	this.clip.inp._x = this.clip.label._x+this.clip.label._width+10;
}

Input.prototype.moveTo = function(x, y)
{
	this.clip._x = x;
	this.clip._y = y;
};

Input.prototype.setText = function(txt)
{
	this.clip.inp.text = txt;
};

Input.prototype.getText = function()
{
	return this.clip.inp.text;
};

Input.prototype.toStrinpg = getInput;


