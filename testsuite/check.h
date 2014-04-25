/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010,
 *   2011, 2012 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 
#ifndef _CHECK_H_
#define _CHECK_H_

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <sstream>
#include <iostream>
#include <string>

#define HAVE_DEJAGNU_H 1 // we ship our own now...
#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"

#define info(x) note x

#else
//#warning "You should install DejaGnu! Using stubs for pass/fail/xpass/xfail..."
class TestState 
{
 public:
  void pass(std::string s) { std::cout << "PASSED: " << s << std::endl;  };
  void xpass(std::string s) { std::cout << "XPASSED: " << s << std::endl;  };
  void fail(std::string s) { std::cout << "FAILED: " << s << std::endl;  };
  void xfail(std::string s) { std::cout << "XFAILED: " << s << std::endl;  };
  void unresolved(std::string s) { std::cout << "UNRESOLVED: " << s << std::endl;  };
};

#define info(x) { printf("NOTE: "); printf x; putchar('\n'); }

#endif

TestState _runtest;

#define check_equals_label(label, expr, expected) \
	{ \
		std::stringstream ss; \
		if ( ! label.empty() ) ss << label << ": "; \
		if ( expr == expected ) \
		{ \
			ss << #expr << " == " << expected; \
			ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
			_runtest.pass(ss.str().c_str()); \
		} \
		else \
		{ \
			ss << #expr << " == '" << expr << "' (expected: " \
				<< expected << ")"; \
			ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
			_runtest.fail(ss.str().c_str()); \
		} \
	}

#define xcheck_equals_label(label, expr, expected) \
	{ \
		std::stringstream ss; \
		if ( label != "" ) ss << label << ": "; \
		if ( expr == expected ) \
		{ \
			ss << #expr << " == " << expected; \
			ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
			_runtest.xpass(ss.str().c_str()); \
		} \
		else \
		{ \
			ss << #expr << " == '" << expr << "' (expected: " \
				<< expected << ")"; \
			ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
			_runtest.xfail(ss.str().c_str()); \
		} \
	}

#define check_equals(expr, expected) check_equals_label(std::string(), expr, expected)

#define xcheck_equals(expr, expected) xcheck_equals_label(std::string(), expr, expected)

#define check(expr) \
	{ \
		std::stringstream ss; \
		ss << #expr; \
		ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
		if ( expr ) { \
			_runtest.pass(ss.str().c_str()); \
		} else { \
			_runtest.fail(ss.str().c_str()); \
		} \
	}

#define xcheck(expr) \
	{ \
		std::stringstream ss; \
		ss << #expr; \
		ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
		if ( expr ) { \
			_runtest.xpass(ss.str().c_str()); \
		} else { \
			_runtest.xfail(ss.str().c_str()); \
		} \
	}

int trymain(int argc, char *argv[]);
#define TRYMAIN(runtest) \
int main(int argc, char *argv[]) { \
  try { \
      return trymain(argc, argv);  \
  } catch (std::exception const&  ex) { \
    (runtest).fail(std::string("caught unexpected exception: ") + ex.what()); \
    return 1; \
  } \
}

#endif // _CHECK_H_
