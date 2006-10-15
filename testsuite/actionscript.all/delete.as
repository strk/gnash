rcsid="$Id: delete.as,v 1.3 2006/10/15 02:30:55 rsavoye Exp $";

#include "check.as"

var anObject = new Object();
check(anObject != undefined);
xcheck(delete anObject);
xcheck(anObject == undefined);
check(!delete noObject);

