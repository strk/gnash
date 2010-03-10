//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_ASHANDLERS_H
#define GNASH_ASHANDLERS_H

#include <string>
#include <vector>
#include "SWF.h"


// Forward declarations
namespace gnash {
    class ActionExec;
    class as_environment;
}

namespace gnash {

namespace SWF { // gnash::SWF

enum ArgumentType {
    ARG_NONE = 0,
    ARG_STR,
    // default hex dump, in case the format is unknown or unsupported
    ARG_HEX,
    ARG_U8,
    ARG_U16,
    ARG_S16,
    ARG_PUSH_DATA,
    ARG_DECL_DICT,
    ARG_FUNCTION2
};


class ActionHandler
{
    typedef void (*ActionCallback)(ActionExec& thread);

public:

    ActionHandler();
    ActionHandler(ActionType type, ActionCallback func);
    ActionHandler(ActionType type, std::string name, 
                  ActionCallback func);
    ActionHandler(ActionType type, std::string name, 
                  ActionCallback func, ArgumentType format);

    /// Execute the action
    void execute(ActionExec& thread) const;

    void toggleDebug(bool state) const { _debug = state; }
    ActionType getType()   const { return _type; }
    std::string getName()   const { return _name; }
    ArgumentType getArgFormat() const { return _arg_format; }

private:
    ActionType _type;
    std::string _name;
    ActionCallback _callback;
    mutable bool _debug;
    ArgumentType _arg_format;
};

/// A singleton containing the supported SWF Action handlers.
class SWFHandlers
{
public:

	// Indexed by action id
	typedef std::vector<ActionHandler> container_type;

	/// Return the singleton instance of SWFHandlers class
	static const SWFHandlers& instance();

	/// Execute the action identified by 'type' action type
	void execute(ActionType type, ActionExec& thread) const;

	void toggleDebug(bool state) { _debug = state; }

	size_t size() const { return get_handlers().size(); }

	ActionType lastType() const
	{
		return ACTION_GOTOEXPRESSION;
	}

	const ActionHandler &operator[] (ActionType x) const
	{
		return get_handlers()[x];
	}

	const char* action_name(ActionType x) const;

private:

	static container_type & get_handlers();

	bool _debug;

	// Use the ::instance() method to get a reference
	SWFHandlers();

	// You won't destroy a singleton
	~SWFHandlers();

};


} // namespace SWF
} // namespace gnash

#endif
