// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_function.h"
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

    GlobalCode(const action_buffer& nBuffer, boost::intrusive_ptr<DisplayObject> nTarget)
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
        if ( ! target->unloaded() )
        {
            ActionExec exec(buffer, target->get_environment());
            exec();
        }
        else
        {
            //log_debug("Sprite %s unloaded, won't execute global code in it", target->getTargetPath().c_str());
        }
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///  - the action target (target)
    ///
    virtual void markReachableResources() const
    {
        if ( target ) target->setReachable();
    }
#endif // GNASU_USE_GC

private:

    const action_buffer& buffer;

    boost::intrusive_ptr<DisplayObject> target;
};

/// Event code 
class EventCode: public ExecutableCode {

public:

    typedef std::vector<const action_buffer*> BufferList;

    EventCode(boost::intrusive_ptr<DisplayObject> nTarget)
        :
        _target(nTarget)
    {}

    EventCode(boost::intrusive_ptr<DisplayObject> nTarget, const BufferList& buffers)
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
        if( ! _target->isDestroyed() )
        {
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
            if( _target->isDestroyed() )  break;

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

    boost::intrusive_ptr<DisplayObject> _target;

    BufferList _buffers;

};

/// Generic event  (constructed by id, invoked using on_event
class QueuedEvent: public ExecutableCode {

public:

    QueuedEvent(boost::intrusive_ptr<DisplayObject> nTarget, const event_id& id)
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
            _target->on_event(_eventId);
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

    boost::intrusive_ptr<DisplayObject> _target;

    const event_id _eventId;

};

/// Function code
class FunctionCode: public ExecutableCode {

public:

    FunctionCode(boost::intrusive_ptr<as_function> nFunc, boost::intrusive_ptr<DisplayObject> nTarget)
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
        as_environment env(getVM(*func)); env.set_target(target.get());
        func->call(fn_call(target.get(), env));
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
        if ( func ) func->setReachable();
        if ( target ) target->setReachable();
    }
#endif // GNASU_USE_GC

private:

    boost::intrusive_ptr<as_function> func;

    boost::intrusive_ptr<DisplayObject> target;
};



} // namespace gnash

#endif // GNASH_EXECUTABLECODE_H
