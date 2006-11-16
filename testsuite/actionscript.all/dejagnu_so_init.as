
// NOTE: when using ming-0.4.0-beta, a bug in 'makeswf' will
//       prevent __shared_assets clip to work (the movieclip
//       will be published with a frame-count of 0, thus
//       actions in it will *NOT* be executed)
//	 We handle this by falling back to 'trace' mode if
//       dejagnu module is not initialized after  'timeout' frames
//	 in a frameloop.

this.timeout = 10; // 10 frames of timeout


// By default 'makeswf' makes the __shared_assets clip invisible,
// make it visible to *see* visual traces
if ( __shared_assets != undefined )
{
	__shared_assets._visible = true;
}
else
{
	trace("__shared_assets undefined: did you run 'makeswf' with -i<path_to_Dejagnu.swf>:dejagnu ?");
}

this.framec = 0;
this.onEnterFrame = function() {
	if ( _root.dejagnu_module_initialized )
	{
		// disable frameloop
		this.onEnterFrame = undefined;

		// setup some dejagnu wrappers
		info = function(msg) {
			xtrace(msg);
			trace(msg);
		};
		pass_check = function (msg) {
			pass(msg);
		};
		xpass_check = function (msg) {
			xpass(msg);
		};
		fail_check = function (msg) {
			fail(msg);
		};
		xfail_check = function (msg) {
			xfail(msg);
		};

		// make __shared_assets visible (in case it didnt' work before)
		__shared_assets._visible = true;

		// jump to next frame
		gotoAndPlay(1);
	}
	else if ( ++this.framec > this.timeout )
	{
		// disable frameloop
		this.onEnterFrame = undefined;

		// complain
		trace("No properly initialized dejagnu module found after "
			+ this.timeout + " frame loops.\n"
			+ " Possible reasons are:\n"
			+ " 1) this testcase was compiled using a bogus\n"
			+ "    makeswf version (up to Ming-0.4.0beta2).\n"
			+ " 2) You are using a player with bogus IMPORT \n"
			+ "    tag handling (actions in the imported movie \n"
			+ "    have not been run yet and we should be in frame2\n"
			+ "    of the importer movie so far).\n"
			+ " 4) The Dejagnu.swf file is corrupted or was not found\n"
			+ "    where expected.\n"
			+ "In any case, we will fallback to trace mode\n\n" );

		// provide fall-back functions
		info = function(msg) {
			trace(msg);
		};
		pass_check = function (msg) {
			trace('PASSED: '+msg);
		};
		xpass_check = function (msg) {
			trace('XPASSED: '+msg);
		};
		fail_check = function (msg) {
			trace('FAILED: '+msg);
		};
		xfail_check = function (msg) {
			trace('XFAILED: '+msg);
		};

		// jump to next frame
		gotoAndPlay(1);
	}
	else
	{
		var retries = this.timeout - this.framec;
		trace("Dejagnu not initialized yet at frame " + framec +" will try again for another frame");
	}
};

stop();
