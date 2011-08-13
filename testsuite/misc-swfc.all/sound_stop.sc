/*
 *   Copyright (C) 2011 Free Software Foundation, Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 */ 

.flash bbox=400x200 filename="sound_test.swf" version=6 fps=1 background=white

.frame 1

    #include "check.sc" // for MEDIA..

    .sound audio1 MEDIA(brokenchord.wav)
    .sound audio2 MEDIA(brokenchord.wav)
    .sound audio3 MEDIA(brokenchord.wav)

    .action:
        createTextField("tf", 0, 0, 0, 0, 0);
        tf.autoSize = true;
        tf.text = "You should hear a three notes chord forming.";
    .end

.frame 2

    .action:
        s1 = new Sound();
        s1.attachSound("audio1");
        s1.start(0, 100);
    .end


.frame 3
    .action:
        s2 = new Sound();
        s2.attachSound("audio2");
        s2.start(0, 100);
    .end

.frame 4
    .action:
        s3 = new Sound();
        s3.attachSound("audio3");
        s3.start(0, 100);
    .end

.frame 9
    .action:
        s4 = new Sound();
        s4.stop();
    .end

.frame 10
    .action:
        tf.text += "\n";
        tf.text += "Now all sounds should be off";
        stop();
    .end

.end
