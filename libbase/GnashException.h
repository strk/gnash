// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#define GNASH_GNASHEXCEPTION_H

#include <stdexcept>
#include <string>
#include "dsodefs.h"

namespace gnash
{

/// Top-level gnash exception
class DSOEXPORT GnashException: public std::runtime_error
{

public:

	GnashException(const std::string& s)
		:
        std::runtime_error(s)
	{}

	GnashException()
		:
        std::runtime_error("Generic error")
	{}

	virtual ~GnashException() throw() {}
};

/// An exception from MediaHandler subsystem
class DSOEXPORT MediaException : public GnashException
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

/// An exception from SoundHandler subsystem
class DSOEXPORT SoundException : public GnashException
{

public:

	SoundException(const std::string& s)
		:
		GnashException(s)
	{}

	SoundException()
		:
		GnashException("Audio error")
	{}

	virtual ~SoundException() throw() {}

};

/// An SWF parsing exception 
class DSOEXPORT ParserException : public GnashException
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

} // namespace gnash

#endif // def GNASH_GNASHEXCEPTION_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
