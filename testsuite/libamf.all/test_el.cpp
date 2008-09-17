// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_DEJAGNU_H

#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <iostream>
#include <string>
#include "as_value.h"
#include "as_object.h"

#include "dejagnu.h"
#include "as_object.h"
#include "arg_parser.h"
#include "buffer.h"
#include "network.h"
#include "amf.h"
#include "element.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

// Prototypes for test cases
static void test_construct();
static void test_destruct();
static void test_make();
static void test_operators();
static void test_properties();

// Enable the display of memory allocation and timing data
static bool memdebug = false;

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();
RcInitFile& rcfile = RcInitFile::getDefaultInstance();

int
main(int argc, char *argv[])
{    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'w', "write",         Arg_parser::no  },
            { 'm', "memstats",      Arg_parser::no  },
            { 'd', "dump",          Arg_parser::no  },
        };
    
    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() ) {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }
    
    for( int i = 0; i < parser.arguments(); ++i ) {
        const int code = parser.code(i);
        try {
            switch( code ) {
              case 'h':
                  usage ();
                  exit(EXIT_SUCCESS);
              case 'v':
                    dbglogfile.setVerbosity();
                    // This happens once per 'v' flag 
                    log_debug(_("Verbose output turned on"));
                    break;
              case 'm':
                    // This happens once per 'v' flag 
                    log_debug(_("Enabling memory statistics"));
                    memdebug = true;
                    break;
              case 'w':
                  rcfile.useWriteLog(true); // dbglogfile.setWriteDisk(true);
                  log_debug(_("Logging to disk enabled"));
                  break;
                  
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }

    // run the tests
    test_construct();
    test_make();
    test_operators();
    test_destruct();
    test_properties();
}

void
test_properties()
{
    Element top;
    top.makeObject("app");

    Element *prop1 = new Element;
    prop1->makeString("property one");
    top.addProperty(prop1);
    
    Element *prop2 = new Element;
    prop2->makeString("property two");
    top.addProperty(prop2);

    if (top.propertySize() == 2) {
        runtest.pass("Adding property");
    } else {
        runtest.fail("Adding property");
    }
    
//    top.dump();
}

void
test_construct()
{
    // First test the init method, which is all the constructor does anyway.. First
    // we test just making regular elements instead of named elements, ie...
    // AMF "variables".
//     Element el1;
//     if (el1.getType() == Element::NOTYPE) {
//         runtest.pass("Created empty element");
//     } else {
//         runtest.fail("Created empty element");
//     }

    Element el2;
    double dub = 54.3;
    el2.init(dub);
    if ((el2.getType() == Element::NUMBER_AMF0) &&
        (el2.to_number() == dub)) {
        runtest.pass("Initialized as double element");
    } else {
        runtest.fail("Initialized as double element");
    }

    Element el3;
    bool flag = true;
    el3.init(flag);
    if ((el3.getType() == Element::BOOLEAN_AMF0) &&
        (el3.to_bool() == true)) {
        runtest.pass("Initialized as bool element");
    } else {
        runtest.fail("Initialized as bool element");
    }

    Element el4;
    string str = "Hello World";
    el4.init(str);
    if ((el4.getType() == Element::STRING_AMF0) &&
        (el4.getLength() == str.size())) {
        runtest.pass("Initialized as string element");
    } else {
        runtest.fail("Initialized as string element");
    }

    // Now test init with the variable name
    Element el5;
    dub = 2.456;
    el5.init("test1", dub);
    if ((el5.getType() == Element::NUMBER_AMF0) &&
        (strcmp(el5.getName(), "test1") == 0) &&
        (el5.to_number() == dub)) {
        runtest.pass("Initialized as double element with name");
    } else {
        runtest.fail("Initialized as double element with name");
    }

    Element el6;
    flag = true;
    el6.init("test2", flag);
    if ((el6.getType() == Element::BOOLEAN_AMF0) &&
        (strcmp(el6.getName(), "test2") == 0) &&
        (el6.to_bool() == true)) {
        runtest.pass("Initialized as bool element with name");
    } else {
        runtest.fail("Initialized as bool element with name");
    }

    Element el7;
    str = "Hello World";
    el7.init("test3", str);
    if ((el7.getType() == Element::STRING_AMF0) &&
        (strcmp(el7.getName(), "test3") == 0) &&
        (el7.getLength() == str.size())) {
        runtest.pass("Initialized as string element with name");
    } else {
        runtest.fail("Initialized as string element with name");
    }

    // Now test the actual constructor
    dub = 23.45;
    Element elnum1(dub);
    if ((elnum1.getType() == Element::NUMBER_AMF0) &&
        (elnum1.to_number() == dub)) {
        runtest.pass("Constructed as double element");
    } else {
        runtest.fail("Constructed as double element");
    }

    flag = true;
    Element elbool1(flag);
    if ((elbool1.getType() == Element::BOOLEAN_AMF0) &&
        (elbool1.to_bool() == true)) {
        runtest.pass("Constructed as bool element");
    } else {
        runtest.fail("Constructed as bool element");
    }

    str = "Guten Tag";
    Element elstr1(str);
    if ((elstr1.getType() == Element::STRING_AMF0) &&
        (elstr1.getLength() == str.size())) {
        runtest.pass("Constructed as string element");
    } else {
        runtest.fail("Constructed as string element");
    }
    // And now test constrcutors with variable names
    dub = 23.45;
    Element elnum2(dub);
    if ((elnum2.getType() == Element::NUMBER_AMF0) &&
        (elnum2.to_number() == dub)) {
        runtest.pass("Constructed as double element with name");
    } else {
        runtest.fail("Constructed as double element with name");
    }

    flag = true;
    Element elbool2(flag);
    if ((elbool2.getType() == Element::BOOLEAN_AMF0) &&
        (elbool2.to_bool() == true)) {
        runtest.pass("Constructed as bool element with name");
    } else {
        runtest.fail("Constructed as bool element with name");
    }

    str = "Aloha";
    Element elstr2(str);
    if ((elstr2.getType() == Element::STRING_AMF0) &&
        (elstr2.getLength() == str.size())) {
        runtest.pass("Constructed as string element with name");
    } else {
        runtest.fail("Constructed as string element with name");
    }
}

void
test_destruct()
{
}

void
test_make()
{
    Element el1;
    string str = "Hello World!";
    el1.makeString("Hello World!");
    if ((el1.getType() == Element::STRING_AMF0) &&
        (el1.to_string() == str)) {
        runtest.pass("Made string element");
    } else {
        runtest.fail("Made string element");
    }

    Element el2;
    bool sheet = true;
    el2.makeBoolean(sheet);
    if ((el2.getType() == Element::BOOLEAN_AMF0) &&
        (el2.to_bool() == sheet)) {
        runtest.pass("Made bool element");
    } else {
        runtest.fail("Made bool element");
    }

    Element el3;
    el3.clear();
    el3.makeNull();
    if (el3.getType() == Element::NULL_AMF0) {
        runtest.pass("Made NULL Value element");
    } else {
        runtest.fail("Made NULL Value element");
    }

    Element el4;
    el4.makeUndefined();
    if (el4.getType() == Element::UNDEFINED_AMF0) {
        runtest.pass("Made Undefined element");
    } else {
        runtest.fail("Made Undefined element");
    }

    Element el5;
    el5.clear();
    el5.makeObjectEnd();
    if (el5.getType() == Element::OBJECT_END_AMF0) {
        runtest.pass("Made Object End element");
    } else {
        runtest.fail("Made Object End element");
    }

    Element el6;
    el6.clear();
    el6.makeNullString();
    if ((el6.getType() == Element::STRING_AMF0) &&
        (el6.getLength() == 1)) {
        runtest.pass("Made NULL String element");
    } else {
        runtest.fail("Made NULL String element");
    }
    
    double num = 123.456;
    Element el7;
    el7.clear();
    el7.makeNumber(num);
    if ((el7.getType() == Element::NUMBER_AMF0) &&
        (el7.to_number() == num)) {
        runtest.pass("Made double element");
    } else {
        runtest.fail("Made double element");
    }

    Element el8;
    el8.clear();
    el8.makeObject("app");
    if (el8.getType() == Element::OBJECT_AMF0) {
        runtest.pass("Made Object element");
    } else {
        runtest.fail("Made Object element");
    }
    
    Element el9;
    el9.clear();
    el9.makeTypedObject("foobar");
    if (el9.getType() == Element::TYPED_OBJECT_AMF0) {
        runtest.pass("Made Object element");
    } else {
        runtest.fail("Made Object element");
    }
    
    Element el10;
    el10.clear();
    el10.makeECMAArray();
    if (el10.getType() == Element::ECMA_ARRAY_AMF0) {
        runtest.pass("Made ECMA Array Object element");
    } else {
        runtest.fail("Made ECMA Array Object element");
    }
    
    Element el11;
    el11.clear();
    el11.makeMovieClip();
    if (el11.getType() == Element::MOVIECLIP_AMF0) {
        runtest.pass("Made MovieClip Object element");
    } else {
        runtest.fail("Made MovieClip Object element");
    }
    
    Element el12;
    el12.clear();
    el12.makeRecordSet();
    if (el12.getType() == Element::RECORD_SET_AMF0) {
        runtest.pass("Made MovieClip Object element");
    } else {
        runtest.fail("Made MovieClip Object element");
    }
    
    Element el13;
    el13.clear();
    el13.makeReference();
    if (el13.getType() == Element::REFERENCE_AMF0) {
        runtest.pass("Made Reference Object element");
    } else {
        runtest.fail("Made Reference Object element");
    }
    
    Element el14;
    el14.clear();
    el14.makeLongString();
    if (el14.getType() == Element::LONG_STRING_AMF0) {
        runtest.pass("Made Long String Object element");
    } else {
        runtest.fail("Made Long String Object element");
    }
    
    Element el15;
    el15.clear();
    el15.makeUnsupported();
    if (el15.getType() == Element::UNSUPPORTED_AMF0) {
        runtest.pass("Made Unsupported Object element");
    } else {
        runtest.fail("Made Unsupported Object element");
    }
    
    // Test recreating an element as a large size data type.
    Element rel1;
    rel1.clear();
    rel1.makeBoolean(true);
    rel1.makeNumber(num);
    if ((rel1.getType() == Element::NUMBER_AMF0) &&
        (rel1.getLength() == amf::AMF0_NUMBER_SIZE) &&
        (rel1.to_number() == num)) {
        runtest.pass("Remade boolean as a double element");
    } else {
        runtest.fail("Remade boolean as a double element");
    }

// amf::Element::makeNumber(std::string const&, double)
// amf::Element::makeObject(unsigned char*, unsigned int)
// amf::Element::makeString(char const*, unsigned int)
// amf::Element::makeString(unsigned char*, unsigned int)
// amf::Element::makeString(std::string const&, std::string const&)
// amf::Element::makeBoolean(std::string const&, bool)
// amf::Element::makeECMAArray(unsigned char*, unsigned int)
// amf::Element::makeMovieClip(unsigned char*, unsigned int)
// amf::Element::makeRecordSet(unsigned char*, unsigned int)
// amf::Element::makeReference(unsigned char*, unsigned int)
// amf::Element::makeXMLObject(unsigned char*, unsigned int)
// amf::Element::makeLongString(unsigned char*, unsigned int)
// amf::Element::makeStrictArray(unsigned char*, unsigned int)
// amf::Element::makeTypedObject(unsigned char*, unsigned int)
    
}
    
// amf::Element::operator=(amf::Element&)
// amf::Element::operator[](int)
void
test_operators()
{
    Element el1, el2;

    // Test equivalance operators
    if (el1 == el2) {
        runtest.pass("Element::operator==(Element &) both empty");
    } else {
        runtest.fail("Element::operator==(Element &) both empty");
    }    

    el1.makeString("Hey Now");
    if (el1 == el2) {
        runtest.fail("Element::operator==(Element &) one empty");
    } else {
        runtest.pass("Element::operator==(Element &) one empty");
    }
    
    el2.makeString("Hey Now");
    if (el1 == el2) {
        runtest.fail("Element::operator==(Element &) neither empty");
    } else {
        runtest.pass("Element::operator==(Element &) neither empty");
    }

    // Test copy operator
    Element el3;
    el3 = el1;
    if (el2 == el3) {
        runtest.fail("Element::operator=(Element &)");
    } else {
        runtest.pass("Element::operator=(Element &)");
    }
}

static void
usage (void)
{
    cerr << "This program tests AMF Element support in the AMF library." << endl
         << endl
         << _("Usage: test_el [options...]") << endl
         << _("  -h,  --help          Print this help and exit") << endl
         << _("  -v,  --verbose       Output verbose debug info") << endl
         << _("  -m,  --memdebug      Output memory statistics") << endl
         << endl;
}

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
  return 0;  
}

#endif

