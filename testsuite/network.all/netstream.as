
//Open a new NetConnection to attach a remote object
var nc = new NetConnection();

//RTMP=(Real-Time Messaging Protocol) 
//rtmp://host[:port]/appName[/instanceName]

//nc.connect("http://localhost");
nc.connect("rtmp://localhost/oflaDemo");

//Declare a Streaming Object called NetStream
var ns = new NetStream(nc);
ns.setBufferTime(10);

// Attach de stream to the video
// ns.attachVideo();

//Push Play on the movie
// NewStream.attachAudio(Microphone.get());
// NewStream.attachVideo(Camera.get()); 

// this is a url variable passed in the object call
//ns.publish(stream);
ns.play("DarkKnight.flv");

note(dumpObject(ns));

           
