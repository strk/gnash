
rcsid="$Id: delete.as,v 1.14 2008/03/11 19:31:48 strk Exp $";
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

//
// Deleting an object's member
//

obj = new Object;
obj.a = 1;
check_equals(obj.a, 1);
check(delete obj.a);
check_equals(typeof(obj.a), 'undefined');
check(!delete obj.a);
check(!delete unexistent.a);


// TODO: test deletion of variables referenced by path (slash-based or dot-based)
//       make sure to test use of 'this' here too !

// --------------------------------------------------------
// Check malformed delete calls. Most of these work anyway.
// --------------------------------------------------------

#if MING_VERSION_CODE >= 00040300

/* Check normal deletes. The first probably uses delete, the second
   probably delete2 */
o = 5;
delete o;
check_equals(o, undefined);

o = {};
o.b = 5;
delete o.b;
check_equals(o.b, undefined);

/* Check deleting a single variable with delete and delete2. This should fail
   with delete */
o = 5;

var reto;

// This tests opcode delete called with only one stack item. The
// result is pushed to a register, then assigned to reto. A normal
// setvariable call needs the variable to be on the stack before the
// value, but the the stack would have two items for the delete call...
asm {
   push 'o'
   delete
   setregister r:3
   push 'reto'
   push r:3
   setvariable
   pop
};

#if OUTPUT_VERSION < 7
 check_equals(o, undefined)
 check_equals(reto, true)
#else
 check_equals(o, 5);
 check_equals(reto, false)
#endif

o = 5;
asm {
   push 'o'
   delete2
   pop
};

check_equals(o, undefined);

/* Check deleting a path with delete and delete2 */
o = {};
o.b = 5;
asm {
   push 'o.b'
   delete
   pop
};
#if OUTPUT_VERSION < 7
 check_equals(o.b, undefined)
#else
 check_equals(o.b, 5);
#endif

o = {};
o.b = 5;
asm {
   push 'o.b'
   delete2
   pop
};

check_equals(o.b, undefined);

/* Check deleting a path with an object on the stack. This should fail
   for delete, but work for delete2 */ 
o = {};
o.b = 5;
asm {
   push 'o'
   getvariable
   push 'o.b'
   delete
   pop
};

check_equals(o.b, 5);

o = {};
o.b = 5;
asm {
   push 'o'
   getvariable
   push 'o.b'
   delete2
   pop
};

check_equals(o.b, undefined);

/* Check deleting a path with a string on the stack. This should fail
   for delete, but work for delete2 */ 
o = {};
o.b = 5;
asm {
   push 'string'
   push 'o.b'
   delete
   pop
};

check_equals(o.b, 5);

o = {};
o.b = 5;
asm {
   push 'string'
   push 'o.b'
   delete2
   pop
};

check_equals(o.b, undefined);

/* Check deleting a path relative to an object on the stack. This should fail
   for both */ 
o = {};
o.b = {};
o.b.c = 5;
asm {
   push 'o'
   getvariable
   push 'b.c'
   delete
   pop
};

check_equals(o.b.c, 5);

o = {};
o.b = {};
o.b.c = 5;
asm {
   push 'o'
   getvariable
   push 'b.c'
   delete2
   pop
};

check_equals(o.b.c, 5);

/* Check deleting a property with getVariable used on a path. This doesn't have
   much to do with delete, but it should work for delete */ 
o = {};
o.b = {};
o.b.c = 5;
asm {
   push 'o.b'
   getvariable
   push 'c'
   delete
   pop
};

check_equals(o.b.c, undefined);

o = {};
o.b = {};
o.b.c = 5;
asm {
   push 'o.b'
   getvariable
   push 'c'
   delete2
   pop
};

check_equals(o.b.c, 5);


_root.e = "hi";

f = function() {
    check_equals(e, "hi");
    var e = "hello";
    check_equals(e, "hello");
    check_equals(_root.e, "hi");

    ret = delete e;
    xcheck_equals(ret, false);
    xcheck_equals(e, "hello");
    check_equals(_root.e, "hi");

    ret = delete e;
    xcheck_equals(ret, false);
    xcheck_equals(e, "hello");
    xcheck_equals(_root.e, "hi");
};

f();
xcheck_equals(_root.e, "hi");
ret = delete e;
xcheck_equals(ret, true);
check_equals(_root.e, undefined);

totals(39+15);

#else

totals(39);

#endif
