rcsid="$Id: delete.as,v 1.9 2007/03/15 22:39:54 strk Exp $";

#include "check.as"

anObject = new Object();
check(anObject != undefined);

anObjectBackup = anObject;
check(delete anObject);
check_equals(typeof(anObject), 'undefined'); 
check_equals(typeof(anObjectBackup), 'object'); 
check(!delete noObject);

//
// Scoped delete (see bug #18482)
//

var anotherObject = new Object();
check(anotherObject);
anotherObject.a = "anotherObject.a";
a = "a";
b = "b";
_global.a = "_global.a";
anotherObject.b = "anotherObject.b (protected)";
ASSetPropFlags(anotherObject, "b", 2); // protect b
with(anotherObject)
{
	check_equals(a, "anotherObject.a");
	check_equals(b, "anotherObject.b (protected)");
	check(!delete b); // protected from deletion !
	check_equals(b, "anotherObject.b (protected)");
	check(delete a);
	check_equals(a, "a");
	check(delete a);

#if OUTPUT_VERSION > 5
	check_equals(a, "_global.a");
	check(delete a);
#else
	check_equals(a, undefined);
	check(! delete a);
#endif

	check_equals(a, undefined);
	check(!delete a);
}

//
// Deleting a user function's prototype
//

function func() {};
func.prototype.appended_value = 4;
check_equals(typeof(func.prototype), 'object');
check_equals(func.prototype.appended_value, 4);
//protoback = MovieClip.prototype;
check(!delete func.prototype);
check_equals(typeof(func.prototype), 'object');
check_equals(func.prototype.appended_value, 4);
