#include "widgets.as"

if ( typeof(url) == 'undefined' )
{
	trace("No 'url' passed in querystring, using 'easysound.mp3'");
	url='easysound.mp3';
}

mySound = new Sound();

mySound.onSoundComplete = function()
{
	trace("soundComplete");
};

s_load = function()
{
	trace("Loading sound "+urlin.getText()+". Streaming ? "+streamingcb.checked());
	mySound.loadSound(urlin.getText(), streamingcb.checked()); 
};

s_start = function()
{
	trace("Starting sound.");
	mySound.start(0, 2); // <startSecs>, <nLoops>
};

s_stop = function()
{
	trace("Stopping sound.");
	mySound.stop(); 
};

streamingcb = new Checkbox(_root, "Streaming");
urlin = new Input(_root, "URL:");
urlin.moveTo(100, 0);
if ( typeof(url) == 'undefined' ) url = 'easysound.mp3';
urlin.setText(url);

loadbtn = new Button(_root, "Load", s_load);
loadbtn.moveTo(0, 50);

startbtn = new Button(_root, "Start", s_start);
startbtn.moveTo(50, 50);

stopbtn = new Button(_root, "Stop", s_stop);
stopbtn.moveTo(100, 50);

