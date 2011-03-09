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
#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include <memory>

#if NPAPI_VERSION == 190
#include "npupp.h"
#else
#include "npapi.h"
#include "npruntime.h"
#include "npfunctions.h"
#endif
#include "pluginbase.h"
#include "dejagnu.h"
#include "../../testsuite/check.h"
#include <regex.h>

#include "external.h"
#include "GnashNPVariant.h"

TestState& runtest = _runtest;

std::map<NPIdentifier, NPVariant *> _properties;
std::map<NPIdentifier,  NPInvokeFunctionPtr> _methods;

int
main(int , char **)
{
    using namespace gnash; 

    NPVariant *value =  (NPVariant *)NPN_MemAlloc(sizeof(NPVariant));

    BOOLEAN_TO_NPVARIANT(true, *value);
    std::string str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<true/>") {
        runtest.pass("convertNPVariant(true)");
    } else {
        runtest.fail("convertNPVariant(true)");
    }
    
    BOOLEAN_TO_NPVARIANT(false, *value);
    str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<false/>") {
        runtest.pass("convertNPVariant(false)");
    } else {
        runtest.fail("convertNPVariant(false)");
    }

    NULL_TO_NPVARIANT(*value);
    str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<null/>") {
        runtest.pass("convertNPVariant(null)");
    } else {
        runtest.fail("convertNPVariant(null)");
    }

    VOID_TO_NPVARIANT(*value);
    str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<void/>") {
        runtest.pass("convertNPVariant(void)");
    } else {
        runtest.fail("convertNPVariant(void)");
    }

    DOUBLE_TO_NPVARIANT(123.456, *value);
    str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<number>123.456</number>") {
        runtest.pass("convertNPVariant(double)");
    } else {
        runtest.fail("convertNPVariant(double)");
    }    

    INT32_TO_NPVARIANT(78, *value);
    str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<number>78</number>") {
        runtest.pass("convertNPVariant(int32)");
    } else {
        runtest.fail("convertNPVariant(int32)");
    }
    
    STRINGZ_TO_NPVARIANT("Hello World!", *value);
    str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<string>Hello World!</string>") {
        runtest.pass("convertNPVariant(string)");
    } else {
        runtest.fail("convertNPVariant(string)");
    }
    
    str = plugin::ExternalInterface::makeProperty("hi", "Hello World!");
    if (str == "<property id=\"hi\">Hello World!</property>") {
        runtest.pass("plugin::ExternalInterface::makeProperty()");
    } else {
        runtest.fail("plugin::ExternalInterface::makeProperty()");
    }
    
#if 0
    ARRAY_TO_NPVARIANT(*value);
    str = plugin::ExternalInterface::convertNPVariant(value);
    if (str == "<array></array>") {
        runtest.pass("convertNPVariant(array)");
    } else {
        runtest.fail("convertNPVariant(array)");
    }
#endif

    std::string prop1 = plugin::ExternalInterface::makeString("foobar");
    std::string prop2 = plugin::ExternalInterface::makeNumber(12.34);
    std::string prop3 = plugin::ExternalInterface::makeNumber(56);
    std::vector<std::string> aargs;
    aargs.push_back(prop1);
    aargs.push_back(prop2);
    aargs.push_back(prop3);
    
    regex_t regex_pat;
    regcomp (&regex_pat, "<array><property id=\"0\"><string>foobar</string></property><property id=\"1\"><number>12.34</number></property><property id=\"2\"><number>56</number></property></array>", REG_NOSUB|REG_NEWLINE);
    str = plugin::ExternalInterface::makeArray(aargs);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(str.c_str()), 0, (regmatch_t *)0, 0)) {    
        runtest.fail("plugin::ExternalInterface::makeArray()");
    } else {
        runtest.pass("plugin::ExternalInterface::makeArray()");
    }

    std::map<std::string, std::string> margs;
    margs["test1"] = prop1;
    margs["test2"] = prop2;
    margs["test3"] = prop3;
    
    str = plugin::ExternalInterface::makeObject(margs);
    std::string xml = "<object><property id=\"test1\"><string>foobar</string></property><property id=\"test2\"><number>12.34</number></property><property id=\"test3\"><number>56</number></property></object>";
    
    regfree (&regex_pat);
    regcomp (&regex_pat, xml.c_str(), REG_NOSUB|REG_NEWLINE);

//    std::cout << str << std::endl;
    if (regexec (&regex_pat, reinterpret_cast<const char*>(str.c_str()), 0, (regmatch_t *)0, 0)) {
        runtest.fail("plugin::ExternalInterface::makeObject()");
    } else {
        runtest.pass("plugin::ExternalInterface::makeObject()");
    }

    //
    // Parsing tests
    //
    xml = "<string>Hello World!</string>";
    GnashNPVariant np = plugin::ExternalInterface::parseXML(xml);
    std::string data = NPStringToString(NPVARIANT_TO_STRING(np.get()));
    if (NPVARIANT_IS_STRING(np.get()) &&
        (data == "Hello World!")) {
        runtest.pass("plugin::ExternalInterface::parseXML(string)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(string)");
    }

    xml = "<number>123.456</number>";
    np = plugin::ExternalInterface::parseXML(xml);
    double num = NPVARIANT_TO_DOUBLE(np.get());
    if (NPVARIANT_IS_DOUBLE(np.get()) &&
        (num == 123.456)) {
        runtest.pass("plugin::ExternalInterface::parseXML(double)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(double)");
    }

    xml = "<number>78</number>";
    np = plugin::ExternalInterface::parseXML(xml);
    int inum = NPVARIANT_TO_INT32(np.get());
    if (NPVARIANT_IS_INT32(np.get()) &&
        (inum == 78)) {
        runtest.pass("plugin::ExternalInterface::parseXML(int32)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(int32)");
    }

    xml = "<true/>";
    np = plugin::ExternalInterface::parseXML(xml);
    bool flag = NPVARIANT_TO_BOOLEAN(np.get());
    if (NPVARIANT_IS_BOOLEAN(np.get()) &&
        (flag == true)) {
        runtest.pass("plugin::ExternalInterface::parseXML(true)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(true)");
    }

    xml = "<false/>";
    np = plugin::ExternalInterface::parseXML(xml);
    flag = NPVARIANT_TO_BOOLEAN(np.get());
    if (NPVARIANT_IS_BOOLEAN(np.get()) &&
        (flag == false)) {
        runtest.pass("plugin::ExternalInterface::parseXML(false)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(false)");
    }

    xml = "<null/>";
    np = plugin::ExternalInterface::parseXML(xml);
    if (NPVARIANT_IS_NULL(np.get())) {
        runtest.pass("plugin::ExternalInterface::parseXML(null)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(null)");
    }

    xml = "<void/>";
    np = plugin::ExternalInterface::parseXML(xml);
    if (NPVARIANT_IS_VOID(np.get())) {
        runtest.pass("plugin::ExternalInterface::parseXML(void)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(void)");
    }

    xml = "<property id=\"0\"><string>foobar</string></property><property id=\"1\"><number>12.34</number></property><property id=\"2\"><number>56</number></property>";
    std::map<std::string, GnashNPVariant> props = plugin::ExternalInterface::parseProperties(xml);
    np = props["0"];
    data = NPStringToString(NPVARIANT_TO_STRING(np.get()));
    if ((props.size() == 3) && (data == "foobar")) {
        runtest.pass("plugin::ExternalInterface::parseProperties()");
    } else {
        runtest.fail("plugin::ExternalInterface::parseProperties()");
    }
    
    xml = "<object><property id=\"test1\"><string>foobar</string></property><property id=\"test2\"><number>12.34</number></property><property id=\"test3\"><number>56</number></property></object>";
    np = plugin::ExternalInterface::parseXML(xml);
    if (NPVARIANT_IS_OBJECT(np.get())) {
        runtest.pass("plugin::ExternalInterface::parseXML(object)");
    } else {
        runtest.fail("plugin::ExternalInterface::parseXML(object)");
    }
    
    std::vector<std::string> iargs;
    str = plugin::ExternalInterface::makeString("barfoo");
    iargs.push_back(str);
    str = plugin::ExternalInterface::makeNumber(135.78);
    iargs.push_back(str);
    
    str = plugin::ExternalInterface::makeInvoke("barbyfoo", iargs);
    xml = "<invoke name=\"barbyfoo\" returntype=\"xml\"><arguments><string>barfoo</string><number>135.78</number></arguments></invoke>";
//    std::cout << str << std::endl;
    regfree (&regex_pat);
    regcomp (&regex_pat, xml.c_str(), REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(str.c_str()), 0, (regmatch_t *)0, 0) == 0) {
        runtest.pass("plugin::ExternalInterface::makeInvoke()");
    } else {
        runtest.fail("plugin::ExternalInterface::makeInvoke()");
    }
    
    xml = "<arguments><string>barfoo</string><number>135.78</number><number>89</number></arguments>";
    std::vector<GnashNPVariant> arguments = plugin::ExternalInterface::parseArguments(xml);
    np = arguments[0];
    str = NPStringToString(NPVARIANT_TO_STRING(np.get()));
    double dub = NPVARIANT_TO_DOUBLE(arguments[1].get());
    int    val = NPVARIANT_TO_INT32(arguments[2].get());
    if ((arguments.size() == 3) && (str == "barfoo")
        && (dub == 135.78) && (val == 89)) {
        runtest.pass("plugin::ExternalInterface::parseArguments()");
    } else {
        runtest.fail("plugin::ExternalInterface::parseArguments()");
    }

    // Parse an invoke message
    xml = "<invoke name=\"barbyfoo\" returntype=\"xml\"><arguments><string>barfoo</string><number>135.78</number></arguments></invoke>";
    boost::shared_ptr<plugin::ExternalInterface::invoke_t> invoke ( plugin::ExternalInterface::parseInvoke(xml) );
    str = NPStringToString(NPVARIANT_TO_STRING(invoke->args[0].get()));
    if ((invoke->name == "barbyfoo") && (invoke->type == "xml")
        && (NPVARIANT_IS_STRING(invoke->args[0].get()))
        && (str == "barfoo")
        && (NPVARIANT_IS_DOUBLE(invoke->args[1].get()))
        && (NPVARIANT_TO_DOUBLE(invoke->args[1].get()) == 135.78)
        ) {
        runtest.pass("plugin::ExternalInterface::parseInvoke()");
    } else {
        runtest.fail("plugin::ExternalInterface::parseInvoke()");
    }

    // Test for bug #31766
    xml = "<invoke name=\"reportFlashTiming\" returntype=\"xml\"><arguments><string>reportFlashTiming</string><object><property id=\"5\"><number>1297286708921</number></property><property id=\"4\"><string>vr</string></p";
    invoke = plugin::ExternalInterface::parseInvoke(xml);
    if ((invoke->name == "reportFlashTiming") && (invoke->type == "xml")
        && invoke->args.empty())
    {
        runtest.pass("plugin::ExternalInterface::parseInvoke() with missing closing invoke tag");
    } else {
        runtest.fail("plugin::ExternalInterface::parseInvoke() with missing closing invoke tag");
    }


    xml = "<invoke name=\"reportFlashTiming\" returntype=\"xml\"><arguments><string>reportFlashTiming</string><object><property id=\"5\"><number>1297326407594</number></property><property id=\"4\"><string>vr</string></property><property id=\"3\"><number>1297326407147</number></property><property id=\"2\"><string>gv</string></property><property id=\"1\"><number>1297326406281</number></property><property id=\"0\"><string>fs</string></property></object><string>34</string><number>2</number><string>AASb6VeOkQtvnu_8</string><string>0</string><string>LNX%2010%2C1%2C999%2C0</string><string>Gnash%20GNU%2FLinux</string></arguments></invoke>";
    invoke = plugin::ExternalInterface::parseInvoke(xml);
    check_equals (invoke->name, "reportFlashTiming");
    check_equals (invoke->type, "xml");
    xcheck_equals (invoke->args.size(), 8);
    //
    check(NPVARIANT_IS_STRING(invoke->args[0].get()));
    str = NPStringToString(NPVARIANT_TO_STRING(invoke->args[0].get()));
    check_equals(str, "reportFlashTiming");
    //
    xcheck(NPVARIANT_IS_OBJECT(invoke->args[1].get()));
    // TODO: check object contents
    //
    xcheck(NPVARIANT_IS_STRING(invoke->args[2].get()));
//    str = NPStringToString(NPVARIANT_TO_STRING(invoke->args[2].get()));
//    check_equals(str, "34");
    //
    xcheck(NPVARIANT_IS_DOUBLE(invoke->args[3].get()));
//    check_equals(NPVARIANT_TO_DOUBLE(invoke->args[3].get()), 2);
    //
    check(NPVARIANT_IS_STRING(invoke->args[4].get()));
    str = NPStringToString(NPVARIANT_TO_STRING(invoke->args[4].get()));
    xcheck_equals(str, "AASb6VeOkQtvnu_8");
    //
    xcheck(NPVARIANT_IS_STRING(invoke->args[5].get()));
//    str = NPStringToString(NPVARIANT_TO_STRING(invoke->args[5].get()));
//    check_equals(str, "0");
    //
    xcheck(NPVARIANT_IS_STRING(invoke->args[6].get()));
//    str = NPStringToString(NPVARIANT_TO_STRING(invoke->args[6].get()));
//    check_equals(str, "LNX%2010%2C1%2C999%2C0");
    //
    xcheck(NPVARIANT_IS_STRING(invoke->args[7].get()));
//    str = NPStringToString(NPVARIANT_TO_STRING(invoke->args[7].get()));
//    check_equals(str, "Gnash%20GNU%2FLinux");


    {
      xml = "<object><property id=\"5\">";
      GnashNPVariant v = plugin::ExternalInterface::parseXML(xml);
      check(NPVARIANT_IS_NULL(v.get()));
    }

    {
      NPVariant val;
      NULL_TO_NPVARIANT(val);
      check(NPVARIANT_IS_NULL(val));
      GnashNPVariant v = val;
      check(NPVARIANT_IS_NULL(v.get()));
    }

    regfree (&regex_pat);
    NPN_MemFree(value);
}

// We have to implement these two memory allocation functions as
// they're used in the code we're testing.
void *
NPN_MemAlloc(uint32_t size)
{
  void * rv = NULL;
  rv = malloc(size);
  return rv;
}

void
NPN_MemFree(void* ptr)
{
  assert(ptr);
  free(ptr);
}

// These are just stubs to get the test case to link standalone.
NPIdentifier
NPN_GetStringIdentifier(const NPUTF8 *)
{
  return 0;
}

nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData *)
{
  return NULL;
}

NPError
NS_PluginGetValue(NPPVariable, void *)
{
  return NPERR_INVALID_INSTANCE_ERROR;
}

NPError
NS_PluginInitialize()
{
  return NPERR_INVALID_INSTANCE_ERROR;
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
NS_DestroyPluginInstance(nsPluginInstanceBase *)
{
}

// Implement minimal properties handling
bool
NPN_SetProperty(NPP, NPObject*, NPIdentifier name,
                     const NPVariant *value)
{
    _properties[name] = const_cast<NPVariant *>(value);
    return true;
}

bool
NPN_GetProperty(NPP, NPObject* , NPIdentifier name,
                     const NPVariant *value)
{
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it == _properties.end()) return false;
    value = it->second;
    return true;
}

bool
NPN_HasProperty(NPP , NPObject* , NPIdentifier name)
{
    std::map<NPIdentifier, NPVariant *>::iterator it;
    it = _properties.find(name);
    if (it != _properties.end()) {
        return true;
    }
    return false;
}

void
NPN_ReleaseVariantValue(NPVariant *variant)
{
    switch(variant->type) {
        case NPVariantType_String:
        {
            NPN_MemFree(const_cast<NPUTF8*>(gnash::GetNPStringChars(NPVARIANT_TO_STRING(*variant))));
            break;
        }
        case NPVariantType_Object:
        {
            NPObject* obj = NPVARIANT_TO_OBJECT(*variant);
            if (obj) {
                NPN_ReleaseObject(obj);
            }
            break;
        }
        default:
        {}
    }
 
    NULL_TO_NPVARIANT(*variant);
}

NPObject*
NPN_RetainObject(NPObject *obj)
{ assert(obj); ++obj->referenceCount; return obj; }


void
NPN_ReleaseObject(NPObject *npobj)
{
    assert(npobj);
    --npobj->referenceCount;
    if (npobj->referenceCount == 0) {
        NPN_MemFree(npobj);
    }
}
// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
