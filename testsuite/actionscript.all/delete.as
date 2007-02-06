rcsid="$Id: delete.as,v 1.8 2007/02/06 11:00:36 strk Exp $";

#include "check.as"

var anObject = new Object();
check(anObject != undefined);
check(delete anObject);
check(anObject == undefined);
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

