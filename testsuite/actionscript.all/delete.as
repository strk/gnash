#include "check.as"

var anObject = new Object();
check(anObject != undefined);
check(delete anObject);
check(anObject == undefined);
check(!delete noObject);

