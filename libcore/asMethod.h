
#ifndef GNASH_AS_METHOD_H
#define GNASH_AS_METHOD_H

#include "log.h"
#include "string_table.h"
#include "Property.h"
#include "namedStrings.h"

#include <list>

// Forward declarations
namespace gnash {
    class CodeStream;
    class Machine;
    class as_object;
    class asClass;
    class abc_function;
}

namespace gnash {

typedef Property asBinding;

/// A class to represent, abstractly, an ActionScript method.
///
/// Methods are unnamed until they are bound to an object.
class asMethod
{
public:
	
    typedef std::list<asClass*> ArgumentList;

	asMethod();

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

	abc_function* getPrototype() { return _prototype; }

	asBinding* getBinding(string_table::key name);

	bool isNative() { return _isNative; }
	bool hasBody() const { return _body != NULL; }

	as_object* construct(as_object* /*base_scope*/) { /* TODO */ return NULL; }

	bool hasActivation();

	CodeStream *getBody() { return _body; }
	void setBody(CodeStream *b) { _body = b; }

	bool addValue(string_table::key name, asNamespace *ns, boost::uint32_t slotID,
		asClass *type, as_value& val, bool isconst);

	bool addSlot(string_table::key name, asNamespace *ns, boost::uint32_t slotID,
		asClass *type);

	bool addMethod(string_table::key name, asNamespace *ns, asMethod *method);

	bool addGetter(string_table::key name, asNamespace *ns, asMethod *method);

	bool addSetter(string_table::key name, asNamespace *ns, asMethod *method);

	bool addMemberClass(string_table::key name, asNamespace *ns,
		boost::uint32_t slotID, asClass *type);
	
	bool addSlotFunction(string_table::key name, asNamespace *ns,
		boost::uint32_t slotID, asMethod *method);

	/// \brief
	/// Set the owner of this method.
	void setOwner(asClass* s);

	/// \brief
	/// Get the unique identifier for the return type. 0 is 'anything'.
	/// (This is the value of any dynamic property.)
	/// Id reference: Type
	asClass* getReturnType() const;

	/// Set the return type
	void setReturnType(asClass* t);

	asMethod *getSuper();

	void setSuper(asMethod* s);

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

	/// \brief
	/// Is the method public?
	bool isPublic() const { return _flags & FLAGS_PUBLIC; }

	/// \brief
	/// Make the method public.
	void setPublic() {
        _flags = (_flags & ~(FLAGS_PRIVATE | FLAGS_PROTECTED)) | FLAGS_PUBLIC;
    }

	/// \brief
	/// How many arguments are required? -1 means unknown.
	int minArgumentCount() const { return _minArguments; }

	/// \brief
	/// Set the required minimum arguments.
	void setMinArgumentCount(int i) { _minArguments = i; }

	/// \brief
	/// How many arguments are allowed? -1 means unknown.
	int maxArgumentCount() const { return _maxArguments; }

	/// Set the required maximum arguments.
	void setMaxArgumentCount(int i) { _maxArguments = i; }

	/// \brief
	/// Push an argument of type t into the method definition
	void pushArgument(asClass *t) { _arguments.push_back(t); }

	/// \brief
	/// Push an optional argument's default value.
	void pushOptional(const as_value& v) { _optionalArguments.push_back(v); }

	/// \brief
	/// Are any of the arguments optional?
	bool optionalArguments() const {
        return minArgumentCount() != maxArgumentCount();
    }

	/// \brief
	/// Get a reference to a list of argument types.
	ArgumentList& getArgumentList() { return _arguments; }

	/// \brief
	/// Get an object capable of executing this function.
	/// Note: This may be NULL, because we might have information about this
	/// function but not actually have it yet.
	as_function* getImplementation() { return _implementation; }

	/// \brief
	/// Print the opcodes that define a method using log_parse.
	void print_body();

	void print_static_constructor(){

	}

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

};

} // namespace gnash

#endif
