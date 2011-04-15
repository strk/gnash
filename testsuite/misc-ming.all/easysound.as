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
 * Simple sound test. Can load (streaming or not) and start/stop.
 * 
 * Initial author: Sandro Santilli <strk@keybit.net>
 *
 ***********************************************************************/

#include "widgets.as"

if ( !hasOwnProperty('url') )
{
	trace("No 'url' passed in querystring, using 'easysound.mp3'");
	url='easysound.mp3';
}

mySound = new Sound();

mySound.onSoundComplete = function()
{
    var s = 'onSoundComplete(';
    for (var i=0; i<arguments.length; ++i) {
        if ( i ) s += ', ';
        s += arguments[i];
    }
    s += ')';
	trace(s);
};

mySound.onLoad = function(success)
{
	var s = "onLoad(";
    for (var i=0; i<arguments.length; ++i) {
        if ( i ) s += ', ';
        s += arguments[i];
    }
    s += ')';
	trace(s);
};

s_load = function()
{
	trace("Loading sound "+urlin.getText()+". Streaming ? "+streamingcb.checked());
	mySound.loadSound(urlin.getText(), streamingcb.checked()); 
};

s_start = function()
{
	var off = offIn.getText();
	var loops = loopsIn.getText();
	trace("Starting sound at seconds offset "+off+" with loop count "+loops);
	mySound.start(off, loops); // <startSecs>, <nLoops>
};

s_stop = function()
{
	trace("Stopping sound.");
	mySound.stop(); 
};

s_getposition = function() {
	//trace("s_getposition called, position is "+mySound.position);
	return mySound.position+"/"+mySound.duration;
};

s_pause = function()
{
	trace("Pausing sound (basically recording current position in offset and stopping.");
	offIn.setText( mySound.position );
	mySound.stop(); 
};

streamingcb = new Checkbox(_root, "Streaming");
urlin = new Input(_root, "URL");
urlin.moveTo(100, 0);
if ( typeof(url) == 'undefined' ) url = 'easysound.mp3';
urlin.setText(url);

offIn = new Input(_root, "Offsets seconds");
offIn.moveTo(0, 30);
offIn.setText(0);

loopsIn = new Input(_root, "Loops");
loopsIn.setText(0);
loopsIn.moveTo(300, 30);

loadbtn = new Button(_root, "Load", s_load);
loadbtn.moveTo(0, 60);

startbtn = new Button(_root, "Start", s_start);
startbtn.moveTo(50, 60);

/*
pausbtn = new Button(_root, "Pause", s_pause);
pausbtn.moveTo(100, 60);
*/

stopbtn = new Button(_root, "Stop", s_stop);
stopbtn.moveTo(100, 60);

infoPosition = new InfoLine(_root, "position", s_getposition);
infoPosition.moveTo(0, 120);

