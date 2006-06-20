rcsid="$Id: delete.as,v 1.2 2006/06/20 20:45:27 strk Exp $";

#include "check.as"

var anObject = new Object();
check(anObject != undefined);
check(delete anObject);
check(anObject == undefined);
check(!delete noObject);

