rcsid="$Id: delete.as,v 1.4 2006/12/09 19:24:47 strk Exp $";

#include "check.as"

var anObject = new Object();
check(anObject != undefined);
check(delete anObject);
check(anObject == undefined);
check(!delete noObject);

