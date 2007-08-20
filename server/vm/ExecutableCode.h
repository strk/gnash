// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

/* $Id: ExecutableCode.h,v 1.8 2007/08/20 15:15:29 strk Exp $ */

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

	virtual ExecutableCode* clone() const=0;

	virtual ~ExecutableCode() {}

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	virtual void markReachableResources() const=0;
#endif // GNASU_USE_GC
};

/// Global code (out of any function)
class GlobalCode: public ExecutableCode {

public:

	GlobalCode(const action_buffer& nBuffer, boost::intrusive_ptr<character> nTarget)
		:
		buffer(nBuffer),
		target(nTarget)
	{}

	ExecutableCode* clone() const
	{
		return new GlobalCode(*this);
	}

	virtual void execute()
	{
		if ( ! target->isUnloaded() )
		{
			ActionExec exec(buffer, target->get_environment());
			exec();
		}
		else
		{
			//log_msg("Sprite %s unloaded, won't execute global code in it", target->getTargetPath().c_str());
		}
	}

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	//
	/// Reachable resources are:
	///	 - the action target (target)
	///
	virtual void markReachableResources() const
	{
		if ( target ) target->setReachable();
	}
#endif // GNASU_USE_GC

private:

	const action_buffer& buffer;

	boost::intrusive_ptr<character> target;
};

/// Event code 
class EventCode: public ExecutableCode {

public:

	typedef vector<const action_buffer*> BufferList;

	EventCode(boost::intrusive_ptr<character> nTarget)
		:
		_target(nTarget)
	{}

	EventCode(boost::intrusive_ptr<character> nTarget, const BufferList& buffers)
		:
		_target(nTarget),
		_buffers(buffers)
	{}


	ExecutableCode* clone() const
	{
		return new EventCode(*this);
	}

	/// Add an action buffer to this event handler
	//
	/// @param buffer
	///	An action buffer to execute. Externally owned
	///	and not copied, so make sure it's kept
	///	alive for the whole EventCode lifetime.
	///
	void addAction(const action_buffer& buffer)
	{
		_buffers.push_back(&buffer);
	}

	virtual void execute()
	{
		// We do want to call the onUnload event handler !!
		//if ( _target->isUnloaded() )
		//{
			//log_msg("Sprite %s unloaded, won't execute global code in it", target->getTargetPath().c_str());
		//	return;
		//}
		for (BufferList::iterator it=_buffers.begin(), itEnd=_buffers.end();
				it != itEnd; ++it)
		{
			ActionExec exec(*(*it), _target->get_environment());
			exec();
		}
	}

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	//
	/// Reachable resources are:
	///	 - the action target (_target)
	///
	virtual void markReachableResources() const
	{
		if ( _target ) _target->setReachable();
	}
#endif // GNASU_USE_GC

private:

	boost::intrusive_ptr<character> _target;

	BufferList _buffers;

};

/// Function code
class FunctionCode: public ExecutableCode {

public:

	FunctionCode(boost::intrusive_ptr<as_function> nFunc, boost::intrusive_ptr<character> nTarget)
		:
		func(nFunc),
		target(nTarget)
	{}

	ExecutableCode* clone() const
	{
		return new FunctionCode(*this);
	}

	virtual void execute()
	{
		as_environment env; env.set_target(target.get());
		func->call(fn_call(target.get(), &env, 0, 0));
	}

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	//
	/// Reachable resources are:
	///	 - the function body (func)
	///	 - the action target (target)
	///
	virtual void markReachableResources() const
	{
		if ( func ) func->setReachable();
		if ( target ) target->setReachable();
	}
#endif // GNASU_USE_GC

private:

	boost::intrusive_ptr<as_function> func;

	boost::intrusive_ptr<character> target;
};



} // namespace gnash

#endif // GNASH_EXECUTABLECODE_H
