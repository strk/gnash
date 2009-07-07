.flash bbox=200x200 filename="sound_test.swf" version=6 fps=30

.box b1 100 100 color=yellow fill=red

.frame 1

    .action:
#include "Dejagnu.sc"
    .end

.sound audio MEDIA(sound1.wav)


.frame 2
	.put b1 pin=center scale=0%
	.action:
		var cubesize=0;
		snd=new Sound();
		snd.attachSound("audio");

		trace("Sound duration is: ");
		trace(snd.duration);
		trace("Correct value should be 13740");
		// fails when no media handler is compiled-in
		check_equals(snd.duration, 13740);
	
		trace("Now I'll get the position before starting it");
		sndpos=snd.position;
		trace("Position is:");
		trace(sndpos);
                check_equals(snd.position, 0);
		snd.start(0,0);
	.end

.frame 6
	.action:
		cubesize+=1;
		b1._width=cubesize;
		b1._height=cubesize;
		if (cubesize==50) {
			cubesize=0;
		}

		if ((snd.position+100)<snd.duration) {
			gotoandPlay(5);
		} else {
			gotoandPlay(8);
		}
	.end

.frame 8
	.action:
		totals(2);
		snd.stop();
	.end
	.stop
.end
