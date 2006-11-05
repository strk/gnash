#ifndef _CHECK_H_
#define _CHECK_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sstream>
#include <iostream>
#include <string>

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#warning "You should install DejaGnu! Using stubs for pass/fail..."
class TestState 
{
 public:
  void pass(std::string s) { std::cout << "PASSED: " << s << std::endl;  };
  void fail(std::string s) { std::cout << "FAILED: " << s << std::endl;  };
};

#endif

TestState _runtest;
#define check_equals_label(label, expr, expected) \
	{ \
		std::stringstream ss; \
		if ( label != "" ) ss << label << ": "; \
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

#define check_equals(expr, expected) check_equals_label("", expr, expected)

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

#endif // _CHECK_H_
