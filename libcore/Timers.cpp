// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software   Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
//
//

#include "Timers.h"
#include "log.h"
#include "fn_call.h"
#include "VM.h"
#include "movie_root.h"
#include "Global_as.h"
#include "as_function.h"

#include <limits> // for numeric_limits
#include <functional>
#include <algorithm>

namespace gnash {

Timer::~Timer()
{
}

Timer::Timer(as_function& method, unsigned long ms,
        as_object* this_ptr, fn_call::Args args, bool runOnce)
    :
    _interval(ms),
    _start(std::numeric_limits<unsigned long>::max()),
    _function(&method),
    _methodName(),
    _object(this_ptr),
    _args(std::move(args)),
    _runOnce(runOnce)
{
    start();
}

Timer::Timer(as_object* this_ptr, ObjectURI methodName,
        unsigned long ms, fn_call::Args args, bool runOnce)
    :
    _interval(ms),
    _start(std::numeric_limits<unsigned long>::max()),
    _function(nullptr),
    _methodName(std::move(methodName)),
    _object(this_ptr),
    _args(std::move(args)),
    _runOnce(runOnce)
{
    start();
}

void
Timer::clearInterval()
{
    _interval = 0;
    _start = std::numeric_limits<unsigned long>::max();
}

void
Timer::start()
{
    _start = getVM(*_object).getTime();
}
    

bool
Timer::expired(unsigned long now, unsigned long& elapsed)
{
    if (cleared()) return false;
    long unsigned expTime = _start + _interval;
    if (now < expTime) return false;
    elapsed = expTime-now;
    return true;
}

void
Timer::executeAndReset()
{
    if (cleared()) return;
    execute();
    if (_runOnce) clearInterval();
    else _start += _interval; // reset the timer
}

void
Timer::execute()
{

    // If _function is not 0, _methodName should be 0 anyway, but the
    // ternary operator is there for clarity.
    as_object* super = _function ? _object->get_super()
                                 : _object->get_super(_methodName);
    VM& vm = getVM(*_object);

    as_value timer_method = _function ? _function :
                                        getMember(*_object, _methodName);

    as_environment env(vm); 

    // Copy args 
    fn_call::Args argsCopy(_args);

    invoke(timer_method, env, _object, argsCopy, super);

}

void
Timer::markReachableResources() const
{
    _args.setReachable();

    if (_function) _function->setReachable();
    if (_object) _object->setReachable();
}

} // namespace gnash
