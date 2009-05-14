
#include "asNamespace.h"
#include "VM.h"
#include "ClassHierarchy.h"

namespace gnash {

void
asNamespace::stubPrototype(string_table::key name)
{
	asClass *pClass = VM::get().getClassHierarchy()->newClass();
	pClass->setName(name);
	addClass(name, pClass);
}

}
