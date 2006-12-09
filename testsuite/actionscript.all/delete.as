rcsid="$Id: delete.as,v 1.5 2006/12/09 19:33:31 strk Exp $";

#include "check.as"

var anObject = new Object();
check(anObject != undefined);
check(delete anObject);
check(anObject == undefined);
check(!delete noObject);

var anotherObject = new Object();
check(anotherObject);
anotherObject.a = "anotherObject.a";
a = "a";
_global.a = "_global.a";

with(anotherObject)
{
	check_equals(a, "anotherObject.a");
	check(delete a);
	check_equals(a, "a");
	check(delete a);
	check_equals(a, "_global.a");
	// it seems our 'delete' thing is failing to seek in _global..
	xcheck(delete a);
	xcheck_equals(a, undefined);
	check(!delete a);
}
