// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_RELAY_H
#define GNASH_RELAY_H

namespace gnash {
    class as_object;
}

namespace gnash {

/// This is the base class for type-specific object data. 
//
/// ActionScript classes with particular type restrictions or type traits
/// should set the Object's _relay member to a subclass of this class.
//
/// The simplest native types, such as Boolean or String, inherit from this
/// type.
class Relay
{
public:
    virtual ~Relay() {};

    /// A Relay itself is not a GC object, but may point to GC resources.
    virtual void setReachable() {}
};


/// A type that requires periodic updates from the core (movie_root).
//
/// Objects with this type of relay can be registered with movie_root, and
/// recieve a callback on every advance.
//
/// This type of Proxy holds a reference to its parent as_object (owner). 
/// If a reference to this ActiveRelay is held by another object,
/// it must be marked reachable so that its owner is not deleted by the GC.
class ActiveRelay : public Relay
{
public:
    ActiveRelay(as_object* owner)
        :
        _owner(owner)
    {}

    /// Make sure we are removed from the list of callbacks on destruction.
    virtual ~ActiveRelay();

    /// ActiveRelay objects must have an advanceState method.
    virtual void update() = 0;

    /// Mark any other reachable resources, and finally mark our owner
    virtual void setReachable();

    as_object& owner() const {
        return *_owner;
    }

protected:

    virtual void markReachableResources() const {}

private:

    /// The as_object that owns this Proxy.
    //
    /// Because we are deleted on destruction of the owner, this pointer will
    /// never be invalid as long as 'this' is valid.
    as_object* _owner;

};

} // namespace gnash

#endif
