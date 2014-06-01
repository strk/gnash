// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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
#include <boost/noncopyable.hpp>

#include "ActionExec.h"
#include "Global_as.h"
#include "fn_call.h"
#include "ConstantPool.h"

namespace gnash {

/// Any executable code 
class ExecutableCode : boost::noncopyable
{
public:

    ExecutableCode(DisplayObject* t) : _target(t) {}

    virtual void execute() = 0;

    virtual ~ExecutableCode() {}

    virtual void setReachable() const {}

    /// Mark reachable resources (for the GC)
    void markReachableResources() const {
        setReachable();
        if (_target) _target->setReachable();
    }

    DisplayObject* target() const { 
        return _target;
    }

private:

    DisplayObject* _target;
};

/// Global code (out of any function)
class GlobalCode : public ExecutableCode
{
public:

    GlobalCode(const action_buffer& nBuffer, DisplayObject* nTarget)
        :
        ExecutableCode(nTarget),
        buffer(nBuffer)
    {}

    virtual void execute() {
        if (!target()->unloaded()) {
            ActionExec exec(buffer, target()->get_environment());
            exec();
        }
    }

private:
    const action_buffer& buffer;
};

/// Event code 
class EventCode : public ExecutableCode
{
public:

    typedef std::vector<const action_buffer*> BufferList;

    EventCode(DisplayObject* nTarget)
        :
        ExecutableCode(nTarget)
    {}

    EventCode(DisplayObject* nTarget, const BufferList& buffers)
        :
        ExecutableCode(nTarget),
        _buffers(buffers)
    {}

    /// Add an action buffer to this event handler
    //
    /// @param buffer
    /// An action buffer to execute. Externally owned
    /// and not copied, so make sure it's kept
    /// alive for the whole EventCode lifetime.
    ///
    void addAction(const action_buffer& buffer) {
        // don't push actions for destroyed DisplayObjects, 
        // our opcode guard is bogus at the moment.
        if (!target()->isDestroyed()) {
            _buffers.push_back(&buffer);
        }
    }

    virtual void execute() {
        for (const action_buffer* buffer : _buffers) {

            // onClipEvents code are guarded by isDestroyed(),
            // still might be also guarded by unloaded()
            if (target()->isDestroyed()) break;

            PoolGuard guard(getVM(target()->get_environment()), nullptr);
            ActionExec exec(*buffer, target()->get_environment(), false);
            exec();
        }
    }

private:
    BufferList _buffers;
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

    DelayedFunctionCall(DisplayObject* target,
            as_object* obj, const ObjectURI& name,
            const as_value& arg1, const as_value& arg2)
        :
        ExecutableCode(target),
        _obj(obj),
        _name(name),
        _arg1(arg1),
        _arg2(arg2)
    {}

    virtual void execute() {
        callMethod(_obj, _name, _arg1, _arg2);
    }

    /// Mark reachable resources (for the GC)
    virtual void setReachable() const {
        _obj->setReachable();
        _arg1.setReachable();
        _arg2.setReachable();
    }

private:
    as_object* _obj;
    ObjectURI _name;
    as_value _arg1, _arg2;
};



} // namespace gnash

#endif // GNASH_EXECUTABLECODE_H
