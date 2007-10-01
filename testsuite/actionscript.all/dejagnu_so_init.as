
// frame loop seems not-working with SWF target 5, while
// setInterval is not working with Gnash, we use both
// to make sure it works :)
//
// Update: setInterval/clearInterval works now, so we
//         don't need the frameloop anymore. We'll keep
//	   the define here just in case we want to make
//	   life easier for other free software players.
//
#define USE_FRAMELOOP

// NOTE: when using ming-0.4.0-beta, a bug in 'makeswf' will
//       prevent __shared_assets clip to work (the movieclip
//       will be published with a frame-count of 0, thus
//       actions in it will *NOT* be executed)
//	 We handle this by falling back to 'trace' mode if
//       dejagnu module is not initialized after  a timeout.

_dejagnu_checker_interval = 200; // milliseconds
// 5 seconds of timeout 
_dejagnu_checker_timeout = 5000/_dejagnu_checker_interval;
_dejagnu_checker_iterations = 0;

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

// Dejagnu initialization checking function
checkIt = function() {
	if ( _root.dejagnu_module_initialized )
	{
		// disable loop
		clearInterval(_dejagnu_checker_interval);
#ifdef USE_FRAMELOOP
		onEnterFrame = undefined;
		delete onEnterFrame;
#endif

		// setup some dejagnu wrappers
		info = function(msg) {
			xtrace(msg);
			trace(msg);
		};
		_root.pass_check = function (msg) {
			_root.pass(msg);
		};
		_root.xpass_check = function (msg) {
			_root.xpass(msg);
		};
		_root.fail_check = function (msg) {
			_root.fail(msg);
		};
		_root.xfail_check = function (msg) {
			_root.xfail(msg);
		};

		// make __shared_assets visible (in case it didnt' work before)
		__shared_assets._visible = true;

		// Print run environment info
		info("SWF" + OUTPUT_VERSION + " - " +
			System.capabilities.version + "\n");

		// jump to next frame
		gotoAndPlay(1);
	}
	else if ( ++_dejagnu_checker_iterations > _dejagnu_checker_timeout )
	{
		// disable loop
		clearInterval(_dejagnu_checker_interval);
#ifdef USE_FRAMELOOP
		this.onEnterFrame = undefined;
#endif

		// complain
		trace("No properly initialized dejagnu module found after "
			+ _dejagnu_checker_timeout + " iterations.\n"
			+ " Possible reasons are:\n"
			+ " 1) this testcase was compiled using a bogus\n"
			+ "    makeswf version (up to Ming-0.4.0-beta1).\n"
			+ " 2) You are using a player with bogus IMPORT \n"
			+ "    tag handling (actions in the imported movie \n"
			+ "    have not been run yet and we should be in frame2\n"
			+ "    of the importer movie so far).\n"
			+ " 4) The Dejagnu.swf file is corrupted or was not found\n"
			+ "    where expected.\n"
			+ "In any case, we will fallback to trace mode\n\n" );

		// provide fall-back functions

		// Track our state
		var _passed = 0;
		var _failed = 0;
		var _xpassed = 0;
		var _xfailed = 0;
		var _untest = 0; 
		var _unresolv = 0;

		info = function(msg) {
			trace(msg);
		};
		pass_check = function (msg) {
			trace('PASSED: '+msg);
			++_passed;
		};
		xpass_check = function (msg) {
			trace('XPASSED: '+msg);
			++_xpassed;
		};
		fail_check = function (msg) {
			trace('FAILED: '+msg);
			++_failed;
		};
		xfail_check = function (msg) {
			trace('XFAILED: '+msg);
			++_xfailed;
		};
		totals = function() {
        		info("Totals:"); 
        		info("    passed: " + _passed ); 
			info("    failed: " + _failed );
			if (_xfailed) {
				info("    expected failures: " + _xfailed);
			}
			if (_xpassed) {
				info("    unexpected passes: " + _xpassed); 
			}
			if (_untest) {
				info("    untested: " + _untest); 
			}
			if (_unresolv) {
				info("    unresolved: " + _unresolv); 
			}
		};

		// Print run environment info
		info("SWF" + OUTPUT_VERSION + " - " +
			System.capabilities.version + "\n");


		// jump to next frame
		gotoAndPlay(1);
	}
	else
	{
		var retries = _dejagnu_checker_timeout - _dejagnu_checker_iterations;
		trace("Dejagnu not initialized yet after " + _dejagnu_checker_iterations +" iterations. Will try again again");
	}
};

// frame loop seems not-working with SWF target 5, while
// setInterval is not working with Gnash, we use both
// to make sure it works :)

_dejagnu_checker_interval = setInterval(checkIt, _dejagnu_checker_interval); 
#ifdef USE_FRAMELOOP
this.onEnterFrame = checkIt;
#endif

stop();
