#include "../actionscript.all/check.as"

// Check that global functions exist when called from SWF5, even when loaded
// in a SWF4.
check_equals(typeof(unescape), "function");
check_equals(typeof(escape), "function");
check_equals(typeof(isNaN), "function");
check_equals(typeof(isFinite), "function");
check_equals(typeof(parseFloat), "function");
check_equals(typeof(parseInt), "function");
check_equals(typeof(NaN), "number");
check_equals(typeof(Infinity), "number");
stop();
