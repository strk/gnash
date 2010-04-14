// 
//   Copyright (C) 2010 Free Software Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

#include "npapi.h"
#include "npruntime.h"
#include "pluginbase.h"
#include "dejagnu.h"

#include "external.h"

TestState runtest;

void* NPN_MemAlloc(uint32_t size)
{
  void * rv = NULL;
  rv = malloc(size);
  return rv;
}

void NPN_MemFree(void* ptr)
{
  free(ptr);
}


int
main(int argc, char *argv[])
{
    ExternalInterface ei;
    NPVariant *value =  (NPVariant *)NPN_MemAlloc(sizeof(NPVariant));

    BOOLEAN_TO_NPVARIANT(true, *value);
    std::string str = ei.convertNPVariant(value);
    if (str == "<true/>") {
        runtest.pass("convertNPVariant(true)");
    } else {
        runtest.fail("convertNPVariant(true)");
    }
    
    BOOLEAN_TO_NPVARIANT(false, *value);
    str = ei.convertNPVariant(value);
    if (str == "<false/>") {
        runtest.pass("convertNPVariant(false)");
    } else {
        runtest.fail("convertNPVariant(false)");
    }

    NULL_TO_NPVARIANT(*value);
    str = ei.convertNPVariant(value);
    if (str == "<null/>") {
        runtest.pass("convertNPVariant(null)");
    } else {
        runtest.fail("convertNPVariant(null)");
    }

    VOID_TO_NPVARIANT(*value);
    str = ei.convertNPVariant(value);
    if (str == "<void/>") {
        runtest.pass("convertNPVariant(void)");
    } else {
        runtest.fail("convertNPVariant(void)");
    }

    DOUBLE_TO_NPVARIANT(123.456, *value);
    str = ei.convertNPVariant(value);
    if (str == "<number>123.456</number>") {
        runtest.pass("convertNPVariant(double)");
    } else {
        runtest.fail("convertNPVariant(double)");
    }    

    INT32_TO_NPVARIANT(78, *value);
    str = ei.convertNPVariant(value);
    if (str == "<number>78</number>") {
        runtest.pass("convertNPVariant(int32)");
    } else {
        runtest.fail("convertNPVariant(int32)");
    }
    
    STRINGZ_TO_NPVARIANT("Hello World!", *value);
    str = ei.convertNPVariant(value);
    if (str == "<string>Hello World!</string>") {
        runtest.pass("convertNPVariant(string)");
    } else {
        runtest.fail("convertNPVariant(string)");
    }
    
#if 0
    ARRAY_TO_NPVARIANT(*value);
    str = ei.convertNPVariant(value);
    if (str == "<array></array>") {
        runtest.pass("convertNPVariant(array)");
    } else {
        runtest.fail("convertNPVariant(array)");
    }

    OBJECT_TO_NPVARIANT(*value);
    str = ei.convertNPVariant(value);
    if (str == "<object></object>") {
        runtest.pass("convertNPVariant(object)");
    } else {
        runtest.fail("convertNPVariant(object)");
    }
#endif
    
}


nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
}

NPError
NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
}

NPError
NS_PluginInitialize()
{
}

void
NS_PluginShutdown()
{
}

char*
NPP_GetMIMEDescription(void)
{
    char *x = 0;
    return x;
}

void
NS_DestroyPluginInstance(nsPluginInstanceBase *aPlugin)
{
}
