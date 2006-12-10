rcsid="$Id: delete.as,v 1.7 2006/12/10 18:39:22 strk Exp $";

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
	check_equals(a, "_global.a");
	check(delete a);
	check_equals(a, undefined);
	check(!delete a);
}

