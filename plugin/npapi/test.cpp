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
#include <map>

#include "npapi.h"
#include "npruntime.h"
#include "pluginbase.h"
#include "npfunctions.h"
#include "dejagnu.h"
#include <regex.h>

#include "external.h"

TestState runtest;

std::map<NPIdentifier, NPVariant *> _properties;
std::map<NPIdentifier,  NPInvokeFunctionPtr> _methods;

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
    
    str = ei.makeProperty("hi", "Hello World!");
    if (str == "<property id=\"hi\">Hello World!</property>") {
        runtest.pass("ExternalInterface::makeProperty()");
    } else {
        runtest.fail("ExternalInterface::makeProperty()");
    }
    
#if 0
    ARRAY_TO_NPVARIANT(*value);
    str = ei.convertNPVariant(value);
    if (str == "<array></array>") {
        runtest.pass("convertNPVariant(array)");
    } else {
        runtest.fail("convertNPVariant(array)");
    }
#endif

    NPObject *obj =  (NPObject *)NPN_MemAlloc(sizeof(NPObject));
    std::string prop1 = ei.makeString("foobar");
    std::string prop2 = ei.makeNumber(12.34);
    std::string prop3 = ei.makeNumber(56);
    std::vector<std::string> aargs;
    aargs.push_back(prop1);
    aargs.push_back(prop2);
    aargs.push_back(prop3);
    
    regex_t regex_pat;
    regcomp (&regex_pat, "<array><property id=\"0\"><string>foobar</string></property><property id=\"1\"><number>12.34</number></property><property id=\"2\"><number>56</number></property></array>", REG_NOSUB|REG_NEWLINE);
    str = ei.makeArray(aargs);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(str.c_str()), 0, (regmatch_t *)0, 0)) {    
        runtest.fail("ExternalInterface::makeArray()");
    } else {
        runtest.pass("ExternalInterface::makeArray()");
    }

    std::map<std::string, std::string> margs;
    margs["test1"] = prop1;
    margs["test2"] = prop2;
    margs["test3"] = prop3;
    
    str = ei.makeObject(margs);
    std::string xml = "<object><property id=\"test1\"><string>foobar</string></property><property id=\"test2\"><number>12.34</number></property><property id=\"test3\"><number>56</number></property></object>";
    
    regcomp (&regex_pat, xml.c_str(), REG_NOSUB|REG_NEWLINE);

//    std::cout << str << std::endl;
    if (regexec (&regex_pat, reinterpret_cast<const char*>(str.c_str()), 0, (regmatch_t *)0, 0)) {
        runtest.fail("ExternalInterface::makeObject()");
    } else {
        runtest.pass("ExternalInterface::makeObject()");
    }

    //
    // Parsing tests
    //
    xml = "<string>Hello World!</string>";
    NPVariant *np = ei.parseXML(xml);
    std::string data = NPVARIANT_TO_STRING(np[0]).UTF8Characters;
    if (NPVARIANT_IS_STRING(*np) &&
        (data == "Hello World!")) {
        runtest.pass("ExternalInterface::parseXML(string)");
    } else {
        runtest.fail("ExternalInterface::parseXML(string)");
    }

    xml = "<number>123.456</number>";
    np = ei.parseXML(xml);
    double num = NPVARIANT_TO_DOUBLE(*np);
    if (NPVARIANT_IS_DOUBLE(*np) &&
        (num == 123.456)) {
        runtest.pass("ExternalInterface::parseXML(double)");
    } else {
        runtest.fail("ExternalInterface::parseXML(double)");
    }

    xml = "<number>78</number>";
    np = ei.parseXML(xml);
    int inum = NPVARIANT_TO_INT32(*np);
    if (NPVARIANT_IS_INT32(*np) &&
        (inum == 78)) {
        runtest.pass("ExternalInterface::parseXML(int32)");
    } else {
        runtest.fail("ExternalInterface::parseXML(int32)");
    }

    xml = "<true/>";
    np = ei.parseXML(xml);
    bool flag = NPVARIANT_TO_BOOLEAN(*np);
    if (NPVARIANT_IS_BOOLEAN(*np) &&
        (flag == true)) {
        runtest.pass("ExternalInterface::parseXML(true)");
    } else {
        runtest.fail("ExternalInterface::parseXML(true)");
    }

    xml = "<false/>";
    np = ei.parseXML(xml);
    flag = NPVARIANT_TO_BOOLEAN(*np);
    if (NPVARIANT_IS_BOOLEAN(*np) &&
        (flag == false)) {
        runtest.pass("ExternalInterface::parseXML(false)");
    } else {
        runtest.fail("ExternalInterface::parseXML(false)");
    }

    xml = "<null/>";
    np = ei.parseXML(xml);
    if (NPVARIANT_IS_NULL(*np)) {
        runtest.pass("ExternalInterface::parseXML(null)");
    } else {
        runtest.fail("ExternalInterface::parseXML(null)");
    }

    xml = "<void/>";
    np = ei.parseXML(xml);
    if (NPVARIANT_IS_VOID(*np)) {
        runtest.pass("ExternalInterface::parseXML(void)");
    } else {
        runtest.fail("ExternalInterface::parseXML(void)");
    }

    xml = "<property id=\"0\"><string>foobar</string></property><property id=\"1\"><number>12.34</number></property><property id=\"2\"><number>56</number></property>";
    std::map<std::string, NPVariant *> props = ei.parseProperties(xml);
    np = props["0"];
    data = NPVARIANT_TO_STRING(np[0]).UTF8Characters;
    if ((props.size() == 3) && (data == "foobar")) {
        runtest.pass("ExternalInterface::parseProperties()");
    } else {
        runtest.fail("ExternalInterface::parseProperties()");
    }
    
    xml = "<object><property id=\"test1\"><string>foobar</string></property><property id=\"test2\"><number>12.34</number></property><property id=\"test3\"><number>56</number></property></object>";
    np = ei.parseXML(xml);
    if (NPVARIANT_IS_OBJECT(*np)) {
        runtest.pass("ExternalInterface::parseXML(object)");
    } else {
        runtest.fail("ExternalInterface::parseXML(object)");
    }

}

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

// These are just stubs to get the test case to link standalone.
NPIdentifier NPN_GetStringIdentifier(const NPUTF8 *name)
{
}

bool NPN_SetProperty(NPP npp, NPObject* obj, NPIdentifier name,
                     const NPVariant *value)
{
    _properties[name] = const_cast<NPVariant *>(value);
}

bool NPN_GetProperty(NPP npp, NPObject* obj, NPIdentifier name,
                     const NPVariant *value)
{
    return _properties[name];
}

bool NPN_HasProperty(NPP npp, NPObject* obj, NPIdentifier name,
                     const NPVariant *value)
{
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        return true;
    }
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
