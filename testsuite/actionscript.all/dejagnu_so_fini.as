// NOTE: when using ming-0.4.0-beta, a bug in 'makeswf' will
//       prevent __shared_assets clip to work (the movieclip
//       will be published with a frame-count of 0, thus
//       actions in it will *NOT* be executed)

// Print totals
//info();
//totals();

// Since we movies importing Dejagnu.swf need
// to be multiframed (actions in first frame will
// be executed *before* actions in imported Dejagnu.swf)
// we need a stop here, or we'll keep looping.
stop();
