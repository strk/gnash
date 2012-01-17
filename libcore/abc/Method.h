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

#ifndef GNASH_AS_METHOD_H
#define GNASH_AS_METHOD_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "string_table.h"
#include "AbcBlock.h"

#include <map>
#include <vector>
#include <list>

// Forward declarations
namespace gnash {
    namespace abc {
        class Machine;
        class abc_function;
        class Namespace;
        class Class;
    }
    class CodeStream;
    class as_object;
}

namespace gnash {
namespace abc {

typedef Property asBinding;

/// A class to represent, abstractly, an ActionScript method.
///
/// Methods are unnamed until they are bound to an object.
class Method
{
public:
	
    typedef std::list<Class*> ArgumentList;

	Method();

    boost::uint32_t methodID() const {
        return _methodID;
    }

    void setMethodID(boost::uint32_t m) {
        _methodID = m;
    }

	void initPrototype(Machine* machine);

	boost::uint32_t getMaxRegisters() { return _maxRegisters;}

	void setMaxRegisters(boost::uint32_t maxRegisters) { 
        _maxRegisters = maxRegisters;
    }

	boost::uint32_t getBodyLength(){ return _bodyLength;}

	void setBodyLength(boost::uint32_t length){ _bodyLength = length;}

    void setMaxStack(boost::uint32_t max) {
        _maxStack = max;
    }
 
    boost::uint32_t maxStack() const {
        return _maxStack;
    }

    void setMaxScope(boost::uint32_t max) {
        _maxScope = max;
    }
 
    boost::uint32_t maxScope() const {
        return _maxScope;
    }
    
    void setScopeDepth(boost::uint32_t depth) {
        _scopeDepth = depth;
    }
 
    boost::uint32_t scopeDepth() const {
        return _scopeDepth;
    }

    abc_function* getPrototype() { return _prototype; }

    /// Add a Trait to this Method.
    void addTrait(const Trait& t) {
        _traits.push_back(t);
    }


    /// Initialize Traits. This is bogus.
    //
    /// TODO: fix!
    void initTraits(AbcBlock& bl);

	asBinding* getBinding(string_table::key name);

	bool isNative() { return _isNative; }
	bool hasBody() const { return _body != NULL; }

	as_object* construct(as_object* /*base_scope*/) {
        // TODO:
        return NULL;
    }

	bool needsActivation() const {
        return _needsActivation;
    }

    void setNeedsActivation() {
        _needsActivation = true;
    }

	CodeStream *getBody() { return _body; }
	void setBody(CodeStream *b) { _body = b; }

	bool addValue(string_table::key name, Namespace *ns,
            boost::uint32_t slotID, Class *type, as_value& val, bool isconst);

	bool addSlot(string_table::key name, Namespace *ns,
            boost::uint32_t slotID, Class *type);

	bool addMethod(string_table::key name, Namespace *ns, Method *method);

	bool addGetter(string_table::key name, Namespace *ns, Method *method);

	bool addSetter(string_table::key name, Namespace *ns, Method *method);

	bool addMemberScript(string_table::key name, Namespace *ns,
		boost::uint32_t slotID, Class *type);
	
	bool addSlotFunction(string_table::key name, Namespace *ns,
		boost::uint32_t slotID, Method *method);

	/// \brief
	/// Set the owner of this method.
	void setOwner(Class* s);

	/// \brief
	/// Get the unique identifier for the return type. 0 is 'anything'.
	/// (This is the value of any dynamic property.)
	/// Id reference: Type
	Class* getReturnType() const;

	/// Set the return type
    //
    /// TODO: This is currently a no-op, so find out what it's for and
    /// implement it.
    /// NB: the return type of a method can be * (any) or void, neither of
    /// which are known names, so this may not be an appropriate way to
    /// handle return type.
	void setReturnType(Class* t);

	Method *getSuper();

	void setSuper(Method* s);

	/// \brief
	/// Is the method final? If so, it may not be overridden.
	bool isFinal() const { return _flags & FLAGS_FINAL; }

	/// \brief
	/// Set the method as final.
	void setFinal() { _flags = _flags | FLAGS_FINAL; }

	/// \brief
	/// Unset the method as final. Not final anymore.
	void unsetFinal() { _flags = _flags & ~FLAGS_FINAL; }

	/// \brief
	/// Is the method private?
	bool isPrivate() const { return _flags & FLAGS_PRIVATE; }

	/// \brief
	/// Make the method private.
	void setPrivate() {
        _flags = (_flags & ~(FLAGS_PUBLIC | FLAGS_PROTECTED)) | FLAGS_PRIVATE;
    }

	/// \brief
	/// Is the method protected?
	bool isProtected() const {
        return _flags & FLAGS_PROTECTED;
    }

	/// \brief
	/// Make the method protected.
	void setProtected() {
        _flags = (_flags & ~(FLAGS_PUBLIC | FLAGS_PRIVATE)) | FLAGS_PROTECTED; }

	/// Is the method public?
	bool isPublic() const { return _flags & FLAGS_PUBLIC; }

	/// Make the method public.
	void setPublic() {
        _flags = (_flags & ~(FLAGS_PRIVATE | FLAGS_PROTECTED)) | FLAGS_PUBLIC;
    }

	/// How many arguments are required? -1 means unknown.
	int minArgumentCount() const { return _minArguments; }

	/// Set the required minimum arguments.
	void setMinArgumentCount(int i) { _minArguments = i; }

	/// How many arguments are allowed? -1 means unknown.
	int maxArgumentCount() const { return _maxArguments; }

	/// Set the required maximum arguments.
	void setMaxArgumentCount(int i) { _maxArguments = i; }

	/// Push an argument of type t into the method definition
    //
    /// A value of 0 stands for 'any'.
	void pushArgument(Class* t) { _arguments.push_back(t); }

	/// Push an optional argument's default value.
	void pushOptional(const as_value& v) { _optionalArguments.push_back(v); }

	/// Are any of the arguments optional?
	bool optionalArguments() const {
        return minArgumentCount() != maxArgumentCount();
    }

	/// Get a reference to a list of argument types.
    //
    /// NB: Some values may be 0, meaning "any".
	const ArgumentList& getArgumentList() const { return _arguments; }

	/// \brief
	/// Get an object capable of executing this function.
	/// Note: This may be NULL, because we might have information about this
	/// function but not actually have it yet.
	as_function* getImplementation() { return _implementation; }

	/// \brief
	/// Print the opcodes that define a method using log_parse.
	void print_body();

private:

	enum Flag
	{
		FLAGS_FINAL = 0x01,
		FLAGS_PROTECTED = 0x02,
		FLAGS_PUBLIC = 0x04,
		FLAGS_PRIVATE = 0x08
	};

	/// A list of type identifiers
	typedef std::map<string_table::key, asBinding> BindingContainer;

	bool addBinding(string_table::key name, asBinding b);
    
    std::vector<Trait> _traits;
	
    boost::uint32_t _methodID;

    abc_function* _prototype;
	int _minArguments;
	int _maxArguments;
	boost::uint32_t _bodyLength;
	bool _isNative;
	ArgumentList _arguments;
	std::list<as_value> _optionalArguments;
	as_function* _implementation;
	unsigned char _flags;
	CodeStream* _body;
	boost::uint32_t _maxRegisters;

    boost::uint32_t _scopeDepth;
    boost::uint32_t _maxScope;
    boost::uint32_t _maxStack;

    bool _needsActivation;

};

} // namespace abc
} // namespace gnash

#endif
