// makeswf -s400x100 -o test.swf  test.as

createTextField("tf", 50, 10, 10, 10, 10);
tf.autoSize = true;
tf.border = 1;
function log(msg) {
	tf.text += msg;
	tf.text += "\n";
	trace(msg);
}

xml = new XML();
xml.onLoad = function(x) {
	log("Loaded: " + x);
	log("XML: " + this);
};
log("Started root");
xml.load('test.xml');

