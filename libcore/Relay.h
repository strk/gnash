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

#ifndef GNASH_RELAY_H
#define GNASH_RELAY_H

#include <boost/noncopyable.hpp>

namespace gnash {
    class as_object;
}

namespace gnash {

/// This is the base class for type-specific object data. 
//
/// A Relay is a polymorphic object that contains arbitrary native
/// type information. Native functions may access the Relay object, determine
/// its derived type, and change its state.
//
/// The base class does nothing. It provides a virtual function to mark
/// GC resources if necessary in subclasses.
//
/// The simplest native types, such as Boolean or String, inherit from this
/// base class directly. They have no GC resources and simply store a 
/// C++ type such as a boolean, double, or std::string, which native functions
/// can access and manipulate.
//
/// More complex types may derive from a subclass of Relay that provides
/// specific functionality such as updates from the VM.
//
/// An as_object with a non-null _relay member is a native class, as this
/// information cannot be accessed by normal ActionScript functions.
class Relay : boost::noncopyable
{
public:
    virtual ~Relay() = 0;

    /// A Relay itself is not a GC object, but may point to GC resources.
    virtual void setReachable() {}

    /// Handle any cleanup necessary before the Relay is destroyed.
    //
    /// Only the replacement of one Relay by another should cause this
    /// to happen. The cleanup may involve deregistration.
    virtual void clean() {}
};

inline Relay::~Relay()
{
}

/// A native type that requires periodic updates from the core (movie_root).
//
/// Objects with this type of relay can be registered with movie_root, and
/// recieve a callback on every advance.
//
/// This type of Relay holds a reference to its parent as_object (owner). 
/// If a reference to this ActiveRelay is held by another object,
/// it must be marked reachable so that its owner is not deleted by the GC.
//
/// The function setReachable() is called on every GC run. It calls
/// markReachableResources() and marks its owner. 
class ActiveRelay : public Relay
{
public:

    explicit ActiveRelay(as_object* owner)
        :
        _owner(owner)
    {}

    /// Make sure we are removed from the list of callbacks on destruction.
    virtual ~ActiveRelay();

    /// ActiveRelay objects must have an update() method.
    //
    /// The method will be called at the heart-beat frequency.
    ///
    virtual void update() = 0;

    /// Mark any other reachable resources, and finally mark our owner
    //
    /// Do not override this function.
    virtual void setReachable();

    /// Remove the ActiveRelay from movie_root's callback set.
    //
    /// This must be called before the Relay is destroyed!
    virtual void clean();

    /// Return the as_object that this Relay is attached to.
    as_object& owner() const {
        return *_owner;
    }

protected:

    /// Mark any reachable resources other than the owner.
    //
    /// Override this function if the subclass holds references to GC
    /// resources other than the owner.
    virtual void markReachableResources() const {}

private:

    /// The as_object that owns this Relay.
    //
    /// Because we are deleted on destruction of the owner, this pointer will
    /// never be invalid as long as 'this' is valid.
    as_object* _owner;

};

} // namespace gnash

#endif
