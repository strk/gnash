//  Arg_parser - A POSIX/GNU command line argument parser.
//    Copyright (C) 2006, 2007, 2008, 2009, 2010 Antonio Diaz Diaz.
//    Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//
//  Arg_parser reads the arguments in `argv' and creates a number of
//    option codes, option arguments and non-option arguments.
//
//    In case of error, `error' returns a non-empty error message.
//
//    `options' is an array of `struct Option' terminated by an element
//    containing a code which is zero. A null name means a short-only
//    option. A code value outside the unsigned char range means a
//    long-only option.
//
//    Arg_parser normally makes it appear as if all the option arguments
//    were specified before all the non-option arguments for the purposes
//    of parsing, even if the user of your program intermixed option and
//    non-option arguments. If you want the arguments in the exact order
//    the user typed them, call `Arg_parser' with `in_order' = true.
//
//    The argument `--' terminates all options; any following arguments are
//    treated as non-option arguments, even if they begin with a hyphen.
//
//    The syntax for optional option arguments is `-<short_option><argument>'
//    (without whitespace), or `--<long_option>=<argument>'.

// This class has been modified with a templated parser.argument<>
// method, allowing typesafe handling of different return types, and
// saving using strto* on the user side. I've added an exception class
// because I'd like to know if we call an argument outside the range
// of argument - there's no reasonable situation in which that would 
// happen. <bwy>

#include "dsodefs.h"
#include <vector>
#include <sstream>

class Arg_parser
{
public:
    enum Has_arg { no, yes, maybe };
    
    struct Option
    {
	int code;			// Short option letter or code ( code != 0 )
	const char * name;		// Long option name (maybe null)
	Has_arg has_arg;
    };
    
    class ArgParserException : public std::exception
    {
    public:
	ArgParserException(const std::string& s)
	    :
	    _msg(s)
	{}
	
	virtual ~ArgParserException() throw() {}
	
	const char* what() const throw() { return _msg.c_str(); }
	
    private:
	
	std::string _msg;
    };
    
private:
    struct Record
    {
	int code;
	std::string argument;
	Record( const int c = 0 ) : code( c ) {}
    };
    
    std::string _error;
    std::vector< Record > data;
    
    bool parse_long_option( const char * const opt, const char * const arg,
			    const Option options[], int & argind ) throw();
    bool parse_short_option( const char * const opt, const char * const arg,
			     const Option options[], int & argind ) throw();
    
public:
    DSOEXPORT Arg_parser( const int argc, const char * const argv[],
			  const Option options[], const bool in_order = false ) throw();
    
    // Restricted constructor. Parses a single token and argument (if any)
    DSOEXPORT Arg_parser( const char * const opt, const char * const arg,
			  const Option options[] ) throw();
    
    const std::string & error() const throw() { return _error; }
    
    // The number of arguments parsed (may be different from argc)
    int arguments() const throw() { return data.size(); }
    
    // If code( i ) is 0, argument( i ) is a non-option.
    // Else argument( i ) is the option's argument (or empty).
    int code( const int i ) const throw()
    {
	if( i >= 0 && i < arguments() ) return data[i].code;
	else return 0;
    }
    
    std::string argument(const int i) const throw(ArgParserException)
    {
	if( i >= 0 && i < arguments() ) return data[i].argument;
	else return _error;
    }    
    
    template<typename T>
    T argument(const int i) const throw (ArgParserException)
    {
        T t = 0;
        if( i >= 0 && i < arguments() )
	    {
		std::istringstream in(data[i].argument);
		in >> t;
		return t;
	    }
        else throw ArgParserException("Code out of range");
    }  
};


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
