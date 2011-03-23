// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#include "Namespace.h"
#include "VM.h"
#include "ClassHierarchy.h"
#include "string_table.h"
#include "log.h"

#include <sstream>

namespace gnash {
namespace abc {

void
Namespace::stubPrototype(ClassHierarchy& ch, string_table::key name)
{
	abc::Class *pClass = ch.newClass();
	pClass->setName(name);
	addScript(name, pClass);
}

void
Namespace::dump(string_table& st)
{
    std::ostringstream s;

    for (container::const_iterator i = _scripts.begin(), e = _scripts.end();
            i != e; ++i)
    {
        const string_table::key t = i->second->getName();
        s << st.value(t) << "(URI: " << t << "), ";
    }

    log_debug("Classes in namespace %s (URI: %s): %s",
            st.value(_uri), _uri, s.str());
}

} // namespace abc
} // namespace gnash
