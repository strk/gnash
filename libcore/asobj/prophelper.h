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
//

// A few defines to make life easier on standard properties.
// #include this where you need it, not at the top.
// First invoke: start_properties(phelp_helper, phelp_class) with
// the helper class (adapter) name and the object class name.
// sp_type -- the C++ type of the member
// sp_convert -- the conversion function from as_value
// sp_name -- the name of the property.
//   Defines a function in phelp_helper with sp_name##_gs as its name
//   that depends on m_##sp_name being the member of the class which
//   corresponds to the sp_name property in ActionScript.
//   See BlurFilter_as.cpp for usage.
//
#ifdef phelp_done
#undef phelp_helper
#undef phelp_property
#undef phelp_array_property
#undef phelp_base_def
#undef phelp_base_imp
#undef phelp_gs
#undef phelp_gs_attach
#undef phelp_gs_attach_begin
#undef phelp_gs_attach_end
#undef phelp_gs_attach_empty
#undef phelp_i
#undef phelp_i_attach
#undef phelp_i_attach_begin
#undef phelp_i_attach_end
#undef phelp_i_attach_empty
#else /* phelp_done */
#ifdef phelp_helper

#define phelp_property(sp_type, sp_convert, sp_name) \
as_value \
phelp_helper::sp_name##_gs(const fn_call& fn) \
{ \
    boost::intrusive_ptr<phelp_helper> ptr = ensureType<phelp_helper>(fn.this_ptr); \
\
    if (fn.nargs == 0) /* getter */ \
    { \
        return as_value(ptr->m_##sp_name ); \
    } \
    /* setter */ \
    sp_type sp_##sp_name = fn.arg(0).to_##sp_convert (); \
    ptr->m_##sp_name = sp_##sp_name; \
\
    return as_value(); \
}

#define phelp_array_property(sp_name) \
as_value \
phelp_helper::sp_name##_gs(const fn_call& fn) \
{ \
    boost::intrusive_ptr<phelp_helper> ptr = ensureType<phelp_helper>(fn.this_ptr); \
    return as_value(); \
}
#if 0
    boost::intrusive_ptr<phelp_helper> ptr = ensureType<phelp_helper>(fn.this_ptr); \
\
    if (fn.nargs == 0) /* getter */ \
    { \
        boost::intrusive_ptr<as_object> tmp = ensureType<as_object>(ptr->m_##sp_name); \
        return as_value(tmp); \
    } \
    /* setter */ \
    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object(); \
    boost::intrusive_ptr<as_array_object> ap = ensureType<as_array_object>(obj); \
    ptr->m_##sp_name = ap; \
\
    return as_value(); \
}
#endif /* 0 */

#define phelp_base_def \
public: \
    phelp_helper(as_object *obj) : as_object(obj) { return; } \
    static as_object* Interface(); \
    static void attachInterface(as_object& o); \
    static void attachProperties(as_object& o); \
\
    static void registerCtor(as_object& global); \
\
    static as_value ctor(const fn_call& fn); \
private: \
    static boost::intrusive_ptr<as_object> s_interface; \
    static boost::intrusive_ptr<builtin_function> s_ctor

#define phelp_base_imp(sp_interface, sp_regname) \
boost::intrusive_ptr<as_object> phelp_helper::s_interface; \
boost:: intrusive_ptr<builtin_function> phelp_helper::s_ctor; \
\
as_object* \
phelp_helper::Interface() \
{ \
    if (phelp_helper::s_interface == NULL) \
    { \
        phelp_helper::s_interface = new as_object sp_interface; \
        VM::get().addStatic(phelp_helper::s_interface.get()); \
\
        phelp_helper::attachInterface(*phelp_helper::s_interface); \
     } \
\
     return phelp_helper::s_interface.get(); \
} \
\
void \
phelp_helper::registerCtor(as_object& global) \
{ \
    if (phelp_helper::s_ctor != NULL) \
        return; \
\
    phelp_helper::s_ctor = new builtin_function(&phelp_helper::ctor, \
        phelp_helper::Interface()); \
    VM::get().addStatic(phelp_helper::s_ctor.get()); \
\
    /* TODO: Is this correct? */ \
    phelp_helper::attachInterface(*phelp_helper::s_ctor); \
\
    global.init_member(#sp_regname , phelp_helper::s_ctor.get()); \
} \
\
void sp_regname##_class_init(as_object& global) \
{ \
    phelp_helper::registerCtor(global); \
} 

#define phelp_i(sp_name) \
    static as_value sp_name(const fn_call& fn)

#define phelp_gs(sp_name) \
    static as_value sp_name##_gs(const fn_call& fn)

#define phelp_gs_attach_begin \
void \
phelp_helper::attachProperties(as_object& o) \
{ \
    boost::intrusive_ptr<builtin_function> gs;

#define phelp_gs_attach_end \
}

#define phelp_gs_attach_empty \
void \
phelp_helper::attachProperties(as_object& /* o */) \
{ \
    return; \
}

#define phelp_gs_attach(sp_name) \
    gs = new builtin_function(phelp_helper::sp_name##_gs, NULL); \
    o.init_property(#sp_name , *gs, *gs)

#define phelp_i_attach_begin \
void \
phelp_helper::attachInterface(as_object& o) \
{ \
    boost::intrusive_ptr<builtin_function> gs; 

#define phelp_i_attach_end \
}

#define phelp_i_attach_empty \
void \
phelp_helper::attachInterface(as_object& /* o */) \
{ \
}

#define phelp_i_attach(sp_name, sp_code_name) \
    o.init_member(#sp_name , new builtin_function(sp_code_name))

#define phelp_i_replace(sp_name, sp_code_name) \
    o.set_member(VM::get().getStringTable().find(#sp_name), new builtin_function(sp_code_name))

#else /* phelp_helper */
#error phelp_helper must be defined.
#endif /* phelp_helper */
#endif /* phelp_done */

