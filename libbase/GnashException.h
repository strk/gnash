// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_GNASHEXCEPTION_H
#define GNASH_GNASHEXCEPTION_H 1

#include <exception>
#include <string>

namespace gnash
{

/// Top-level gnash exception
class GnashException: public std::exception
{

public:

	GnashException(const std::string& s)
		:
		_msg(s)
	{}

	GnashException()
		:
		_msg("Generic error")
	{}

	virtual ~GnashException() throw() {}

	const char* what() const throw() { return _msg.c_str(); }

private:

	std::string _msg;
};

class MediaException : public GnashException
{

public:

	MediaException(const std::string& s)
		:
		GnashException(s)
	{}

	MediaException()
		:
		GnashException("Media error")
	{}

	virtual ~MediaException() throw() {}

};

/// An SWF parsing exception 
class ParserException : public GnashException
{

public:

	ParserException(const std::string& s)
		:
		GnashException(s)
	{}

	ParserException()
		:
		GnashException("Parser error")
	{}

	virtual ~ParserException() throw() {}

};

/// An ActionScript error exception 
class ActionException: public GnashException
{

protected:

	ActionException(const std::string& s)
		:
		GnashException(s)
	{}

	ActionException()
		:
		GnashException("ActionScript error")
	{}

public:

	virtual ~ActionException() throw() {}

};

/// An ActionScript limit exception 
//
/// When this exception is thrown, current execution should
/// be aborted, stacks and registers cleaning included.
///
class ActionLimitException: public ActionException
{

public:

	ActionLimitException(const std::string& s)
		:
		ActionException(s)
	{}

	ActionLimitException()
		:
		ActionException("ActionScript limit hit")
	{}

	virtual ~ActionLimitException() throw() {}

};

/// An ActionScript type error 
//
/// This exception can be thrown by as_value::to_primitive when an object
/// can't be converted to a primitive value or by native function when
/// they are called as method of an unexpected type
///
class ActionTypeError: public ActionException
{

public:

	ActionTypeError(const std::string& s)
		:
		ActionException(s)
	{}

	ActionTypeError()
		:
		ActionException("ActionTypeError")
	{}

	virtual ~ActionTypeError() throw() {}

};

/// An action parsing error, thrown on illegal
/// action buffer access.
class ActionParserException: public ActionException
{

public:

	ActionParserException(const std::string& s)
		:
		ActionException(s)
	{}

	ActionParserException()
		:
		ActionException("Action parser exception")
	{}

	virtual ~ActionParserException() throw() {}

};

/// An unhandled exception in ActionScript, which should
/// interrupt code execution.
class ActionScriptException: public ActionException
{

public:

	ActionScriptException(const std::string& s)
		:
		ActionException(s)
	{}

	ActionScriptException()
		:
		ActionException("Unhandled ActionScript exception")
	{}

	virtual ~ActionScriptException() throw() {}

};


} // namespace gnash

#endif // def GNASH_GNASHEXCEPTION_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
