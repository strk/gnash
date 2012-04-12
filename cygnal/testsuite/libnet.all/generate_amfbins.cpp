// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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
#include <unistd.h>

#include "as_value.h"
#include "as_object.h"

#include "dejagnu.h"
#include "as_object.h"
#include "arg_parser.h"
#include "buffer.h"
#include "network.h"
#include "amf.h"
#include "element.h"

using cygnal::AMF;
using cygnal::Element;
using cygnal::Buffer;
using namespace gnash;
using namespace std;

static void usage (void);

// Prototypes for test cases
static void test_construct();
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

#if 0
    std::vector<boost::shared_ptr<amf::Element> > data1;

    const char *str1 = "property one";
    boost::shared_ptr<amf::Element> prop1(new Element(str1));
    data1.push_back(prop1);

    string str2 = "property two";
    boost::shared_ptr<amf::Element> prop2(new Element(str2));
    data1.push_back(prop2);

    boost::shared_ptr<amf::Element> prop3(new Element("property three"));
    data1.push_back(prop3);

    double num = 123.456;
    boost::shared_ptr<amf::Element> prop4(new Element(num));
    data1.push_back(prop4);
    
    Element top;
    top.makeObject("app", data1);
    
    if ((top.propertySize() == 4)
        && (top.getType() == Element::OBJECT_AMF0)
        && (strcmp(top[0]->to_string(), str1) == 0)
        && (top[1]->to_string() == str2)
        && (strcmp(top[2]->to_string(), "property three") == 0)
        && (top[3]->to_number() == num)) {
        runtest.pass("Made object with properties");
    } else {
        runtest.fail("Made object with properties");
    }
    
    data1.clear();
    top.makeECMAArray(data1);
    if ((top.propertySize() == 4)
        && (top.getType() == Element::ECMA_ARRAY_AMF0)
        && (strcmp(top[0]->to_string(), str1) == 0)
        && (top[1]->to_string() == str2)
        && (strcmp(top[2]->to_string(), "property three") == 0)
        && (top[3]->to_number() == num)) {
        runtest.pass("Made ECMA array");
    } else {
        runtest.fail("Made ECMA array");
    }

    data1.clear();
    top.makeStrictArray(data1);
    if ((top.propertySize() == 4)
        && (top.getType() == Element::STRICT_ARRAY_AMF0)
        && (strcmp(top[0]->to_string(), str1) == 0)
        && (top[1]->to_string() == str2)
        && (strcmp(top[2]->to_string(), "property three") == 0)
        && (top[3]->to_number() == num)) {
        runtest.pass("Made strict array");
    } else {
        runtest.fail("Made strict array");
    }
#endif

    // Test creating number elements. An element with a name is a property.
    double dub = 23.45;
    bool flag = true;
    string str = "Guten Tag";

    Element elnum1(dub);
    boost::shared_ptr<Buffer> bnum1 = cygnal::AMF::encodeElement(elnum1);
    int fd = ::open("amf0-number.bin" ,O_WRONLY|O_CREAT, S_IRWXU);
    ::write(fd, bnum1->reference(), bnum1->allocated()); ::close(fd);

    flag = true;
    Element elbool1(flag);
    boost::shared_ptr<Buffer> bbool1 = cygnal::AMF::encodeElement(elbool1);
    fd = ::open("amf0-boolean.bin" ,O_WRONLY|O_CREAT, S_IRWXU);
    ::write(fd, bbool1->reference(), bbool1->allocated()); ::close(fd);
    
    Element elstr1(str);
    boost::shared_ptr<Buffer> bstr1 = cygnal::AMF::encodeElement(elstr1);
    fd = ::open("amf0-string.bin" ,O_WRONLY|O_CREAT, S_IRWXU);
    ::write(fd, bstr1->reference(), bstr1->allocated()); ::close(fd);

    Element el3;
    el3.clear();
    el3.makeNull();
    boost::shared_ptr<Buffer> bel3 = cygnal::AMF::encodeElement(el3);
    fd = ::open("amf0-null-object.bin" ,O_WRONLY|O_CREAT, S_IRWXU);
    ::write(fd, bel3->reference(), bel3->allocated()); ::close(fd);


    Element el4;
    el4.makeUndefined();
    boost::shared_ptr<Buffer> bel4 = cygnal::AMF::encodeElement(el4);
    fd = ::open("amf0-undefined-object.bin" ,O_WRONLY|O_CREAT, S_IRWXU);
    ::write(fd, bel4->reference(), bel4->allocated()); ::close(fd);

    Element el6;
    el6.clear();
    el6.makeNullString();
    boost::shared_ptr<Buffer> bel6 = cygnal::AMF::encodeElement(el6);
    fd = ::open("amf0-null-string.bin" ,O_WRONLY|O_CREAT, S_IRWXU);
    ::write(fd, bel6->reference(), bel6->allocated()); ::close(fd);    

    Element el15;
    el15.clear();
    el15.makeUnsupported();
    boost::shared_ptr<Buffer> bel15 = cygnal::AMF::encodeElement(el15);
    fd = ::open("amf0-unsupported-object.bin" ,O_WRONLY|O_CREAT, S_IRWXU);
    ::write(fd, bel15->reference(), bel15->allocated()); ::close(fd);


#if 0
    Element el11;
    el11.clear();
    el11.makeMovieClip();
    if (el11.getType() == Element::MOVIECLIP_AMF0) {
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
        (rel1.getDataSize() == cygnal::AMF0_NUMBER_SIZE) &&
        (rel1.to_number() == num)) {
        runtest.pass("Remade boolean as a double element");
    } else {
        runtest.fail("Remade boolean as a double element");
    }
#endif
    

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
main(int /*argc*/, char**)
{
  // nop
  return 0;  
}

#endif

