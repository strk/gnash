/***********************************************************************
 *
 *   Copyright (C) 2005, 2006, 2009, 2010 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 *
 * Set of widget to use in 'makeswf' based quick tests
 * 
 * Initial author: Sandro Santilli <strk@keybit.net>
 *
 ***********************************************************************/

function Widget(where) {
	//trace("Widget ctor called");
	if ( ! arguments.length ) return;
	if ( ! where.hasOwnProperty('nextHighestDepth') ) where.nextHighestDepth=1;
	var d = where.nextHighestDepth++;
	var nam = 'clip'+d;
	this.clip = where.createEmptyMovieClip(nam, d);
}

Widget.prototype.moveTo = function(x, y)
{
	this.clip._x = x;
	this.clip._y = y;
};

function Checkbox(where, label) {

	super(where);

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

Checkbox.prototype = new Widget();

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

// ------------------------------------------------------------------

function Button(where, label, cb)
{
	super(where);

	this.clip.createEmptyMovieClip('eh', 1); // event handler
	this.clip.onRelease = cb;

	this.clip.createTextField('label', 3, 0, 0, 0, 0);
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

Button.prototype = new Widget();

//-------------------

function Input(where, label)
{
	super(where);

	this.clip.createTextField('label', 3, 0, 0, 0, 0, 0);
	this.clip.label.autoSize = true;
	this.clip.label.text = label;

	this.clip.createTextField('inp', 4, 0, 0, 100, 20); // TODO: take as params
	this.clip.inp.autoSize = true;
	this.clip.inp.type = 'input';
	this.clip.inp.border = true;
	//this.clip.inp.text = 'your input here';
	this.clip.inp._x = this.clip.label._x+this.clip.label._width+5;
}

Input.prototype = new Widget();

Input.prototype.setText = function(txt)
{
	this.clip.inp.text = txt;
};

Input.prototype.getText = function()
{
	return this.clip.inp.text;
};

// -------------------------------------------

function InfoLine(where, label, cb)
{
	super(where);

	this.clip.createEmptyMovieClip('eh', 1); // event handler

	this.clip.createTextField('label', 3, 0, 0, 0, 0);
	this.clip.label.autoSize = true;
	this.clip.label.text = label;

	this.clip.createTextField('inp', 4, 0, 0, 100, 20); // TODO: take as params
	this.clip.inp.autoSize = false;
	this.clip.inp.type = 'dynamic';
	this.clip.inp.border = true;
	this.clip.inp.text = 'your input here';
	this.clip.inp._x = this.clip.label._x+this.clip.label._width+5;
	//trace('this clip inp is: '+this.clip.inp);

	this.clip.onEnterFrame = function() {
		info = cb();
		//trace("cb: "+cb()+" inp:"+inp+" clip:"+clip+" this:"+this);
		this.inp.text = info;
	};
}

InfoLine.prototype = new Widget();

InfoLine.prototype.setText = function(txt)
{
	this.clip.inp.text = txt;
};
