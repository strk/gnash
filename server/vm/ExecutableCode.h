// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: ExecutableCode.h,v 1.1 2007/03/27 14:52:05 strk Exp $ */

#ifndef GNASH_EXECUTABLECODE_H
#define GNASH_EXECUTABLECODE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include "smart_ptr.h"
#include "as_function.h"
#include "sprite_instance.h"
#include "ActionExec.h"
#include "fn_call.h"

namespace gnash
{

/// Any executable code 
class ExecutableCode {

public:
	ExecutableCode() {}

	virtual void execute()=0;

	virtual ~ExecutableCode() {}
};

/// Global code (out of any function)
class GlobalCode: public ExecutableCode {

public:

	GlobalCode(const action_buffer& nBuffer, boost::intrusive_ptr<sprite_instance> nTarget)
		:
		buffer(nBuffer),
		target(nTarget)
	{}

	virtual void execute()
	{
		if ( ! target->isUnloaded() )
		{
			ActionExec exec(buffer, target->get_environment());
			exec();
		}
		else
		{
			log_msg("Sprite %s unloaded, won't execute global code in it", target->getTargetPath().c_str());
		}
	}

private:

	const action_buffer& buffer;

	boost::intrusive_ptr<sprite_instance> target;
};

/// Function code
class FunctionCode: public ExecutableCode {

public:

	FunctionCode(boost::intrusive_ptr<as_function> nFunc, boost::intrusive_ptr<sprite_instance> nTarget)
		:
		func(nFunc),
		target(nTarget)
	{}

	virtual void execute()
	{
		//log_msg("Execution of FunctionCode unimplemented yet");
		func->call(fn_call(target.get(), &(target->get_environment()), 0, 0));
	}

private:

	boost::intrusive_ptr<as_function> func;

	boost::intrusive_ptr<sprite_instance> target;
};



} // namespace gnash

#endif // GNASH_EXECUTABLECODE_H
