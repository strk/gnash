// Uncomment the following to test against players w/out trace()
// but with createTextField working
//#define trace xtrace
//#include "xtrace.as"

trace("[Test: movieclip.as]");

// Check some references
trace("   this:"+this);
trace("_parent:"+_parent);
trace("  _root:"+_root);

// Check the _url variable
trace(_url);
trace(this._url);
trace(root._url);

// $Log: movieclip.as,v $
// Revision 1.1  2006/02/01 11:43:16  strk
// Added generic rule to build .swf from .as using makeswf (Ming).
// Changed array.as source to avoid ActionScript2 constructs (class).
// Added initial version of a movieclip AS class tester.
//
