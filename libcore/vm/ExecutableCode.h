// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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


#ifndef GNASH_EXECUTABLECODE_H
#define GNASH_EXECUTABLECODE_H

#include <vector>
#include "smart_ptr.h" // GNASH_USE_GC
#include "as_function.h"
#include "ActionExec.h"
#include "Global_as.h"
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
class GlobalCode : public ExecutableCode {

public:

    GlobalCode(const action_buffer& nBuffer, DisplayObject* nTarget)
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
        if (!target->unloaded()) {
            ActionExec exec(buffer, target->get_environment());
            exec();
        }
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///  - the action target (target)
    ///
    virtual void markReachableResources() const {
        if (target) target->setReachable();
    }
#endif // GNASU_USE_GC

private:

    const action_buffer& buffer;

    DisplayObject* target;
};

/// Event code 
class EventCode : public ExecutableCode {

public:

    typedef std::vector<const action_buffer*> BufferList;

    EventCode(DisplayObject* nTarget)
        :
        _target(nTarget)
    {}

    EventCode(DisplayObject* nTarget, const BufferList& buffers)
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
    /// An action buffer to execute. Externally owned
    /// and not copied, so make sure it's kept
    /// alive for the whole EventCode lifetime.
    ///
    void addAction(const action_buffer& buffer)
    {
        // don't push actions for destroyed DisplayObjects, 
        // our opcode guard is bogus at the moment.
        if (!_target->isDestroyed()) {
            _buffers.push_back(&buffer);
        }
    }

    virtual void execute()
    {
        for (BufferList::iterator it=_buffers.begin(), itEnd=_buffers.end();
                it != itEnd; ++it)
        {
            // onClipEvents code are guarded by isDestroyed(),
            // still might be also guarded by unloaded()
            if (_target->isDestroyed())  break;

            ActionExec exec(*(*it), _target->get_environment(), false);
            exec();
        }
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///  - the action target (_target)
    ///
    virtual void markReachableResources() const
    {
        if ( _target ) _target->setReachable();
    }
#endif // GNASU_USE_GC

private:

    DisplayObject* _target;

    BufferList _buffers;

};

/// Generic event  (constructed by id, invoked using notifyEvent
class QueuedEvent: public ExecutableCode {

public:

    QueuedEvent(DisplayObject* nTarget, const event_id& id)
        :
        _target(nTarget),
        _eventId(id)
    {}


    ExecutableCode* clone() const
    {
        return new QueuedEvent(*this);
    }

    virtual void execute()
    {
        // don't execute any events for destroyed DisplayObject.
        if( !_target->isDestroyed() )
        {
            _target->notifyEvent(_eventId);
        }
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///  - the action target (_target)
    ///
    virtual void markReachableResources() const
    {
        if ( _target ) _target->setReachable();
    }
#endif // GNASU_USE_GC

private:

    DisplayObject* _target;

    const event_id _eventId;

};

/// Function code
class FunctionCode: public ExecutableCode {

public:

    FunctionCode(as_function* nFunc, DisplayObject* nTarget)
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
        as_environment env(getVM(*func)); env.set_target(target);
        func->call(fn_call(getObject(target), env));
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///  - the function body (func)
    ///  - the action target (target)
    ///
    virtual void markReachableResources() const
    {
        if (func) func->setReachable();
        if (target) target->setReachable();
    }
#endif // GNASU_USE_GC

private:

    as_function* func;

    DisplayObject* target;
};

/// This class is used to queue a function call action
//
/// Exact use is to queue onLoadInit, which should be invoked
/// after actions of in first frame of a loaded movie are executed.
/// Since those actions are queued the only way to execute something
/// after them is to queue the function call as well.
///
/// The class might be made more general and accessible outside
/// of the MovieClipLoader class. For now it only works for
/// calling a function with a two argument.
///
class DelayedFunctionCall : public ExecutableCode
{

public:

    DelayedFunctionCall(as_object* target, string_table::key name,
            const as_value& arg1, const as_value& arg2)
        :
        _target(target),
        _name(name),
        _arg1(arg1),
        _arg2(arg2)
    {}


    ExecutableCode* clone() const
    {
        return new DelayedFunctionCall(*this);
    }

    virtual void execute()
    {
        callMethod(_target, _name, _arg1, _arg2);
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///  - the action target (_target)
    ///
    virtual void markReachableResources() const
    {
      _target->setReachable();
      _arg1.setReachable();
      _arg2.setReachable();
    }
#endif // GNASH_USE_GC

private:

    as_object* _target;
    string_table::key _name;
    as_value _arg1, _arg2;

};



} // namespace gnash

#endif // GNASH_EXECUTABLECODE_H
