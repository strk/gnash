// edit_text_character.cpp:  User-editable text regions, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/* $Id: edit_text_character.cpp,v 1.68 2007/06/15 15:00:29 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utf8.h"
#include "log.h"
#include "render.h" // for display()
#include "movie_definition.h" // to extract version info
#include "sprite_instance.h"
#include "edit_text_character.h"
#include "Key.h" // for keyboard events
#include "movie_root.h"	 // for killing focus
#include "as_environment.h" // for parse_path
#include "action.h" // for as_standard_member enum
#include "VM.h"
#include "builtin_function.h" // for getter/setter properties
#include "font.h" // for using the _font member
#include "VM.h"

#include <algorithm>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>

// Text fields have a fixed 2 pixel padding for each side (regardless of border)
#define PADDING_TWIPS 40.0f


namespace gnash {

// Forward declarations
static as_value textfield_get_variable(const fn_call& fn);
static as_value textfield_set_variable(const fn_call& fn);
static as_value textfield_setTextFormat(const fn_call& fn);
static as_value textfield_getTextFormat(const fn_call& fn);
static as_value textfield_setNewTextFormat(const fn_call& fn);
static as_value textfield_getNewTextFormat(const fn_call& fn);
static as_value textfield_addListener(const fn_call& fn);
static as_value textfield_removeListener(const fn_call& fn);

static as_value textfield_getDepth(const fn_call& fn);
static as_value textfield_getFontList(const fn_call& fn);
static as_value textfield_removeTextField(const fn_call& fn);
static as_value textfield_replaceSel(const fn_call& fn);
static as_value textfield_replaceText(const fn_call& fn);
static as_object* getTextFieldInterface();


//
// TextField interface functions
//

static as_value
textfield_get_variable(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);

	return as_value(text->get_variable_name());

}

static as_value
textfield_set_variable(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);

	assert ( fn.nargs > 0 );
	const std::string& newname = fn.arg(0).to_string(&fn.env());

	text->set_variable_name(newname);

	return as_value();
}

static as_value
textfield_setTextFormat(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.setTextFormat()");
		warned = true;
	}

	return as_value();

}

static as_value
textfield_addListener(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.addListener()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_removeListener(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.removeListener()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_setNewTextFormat(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.setNewTextFormat()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_getDepth(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.getDepth()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_getFontList(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.getFontList()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_getNewTextFormat(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.getNewTextFormat()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_getTextFormat(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.getTextFormat()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_replaceSel(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.replaceSel()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_replaceText(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.replaceText()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_removeTextField(const fn_call& fn)
{
	boost::intrusive_ptr<edit_text_character> text = ensureType<edit_text_character>(fn.this_ptr);
	UNUSED(text);

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("TextField.removeTextField()");
		warned = true;
	}

	return as_value();
}

static as_value
textfield_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new as_object(getTextFieldInterface());
	return as_value(obj);
}

//
// TextField interface initialization
//

static void
attachTextFieldInterface(as_object& o)
{
	int target_version = o.getVM().getSWFVersion();

	// SWF5 or higher
	if ( target_version  < 6 ) return;

	// SWF6 or higher
	boost::intrusive_ptr<builtin_function> variable_getter(new builtin_function(&textfield_get_variable, NULL));
	boost::intrusive_ptr<builtin_function> variable_setter(new builtin_function(&textfield_set_variable, NULL));
	o.init_property("variable", *variable_getter, *variable_setter);
	o.init_member("setTextFormat", new builtin_function(textfield_setTextFormat));
	o.init_member("getTextFormat", new builtin_function(textfield_getTextFormat));
	o.init_member("addListener", new builtin_function(textfield_addListener));
	o.init_member("removeListener", new builtin_function(textfield_removeListener));
	o.init_member("setNewTextFormat", new builtin_function(textfield_setNewTextFormat));
	o.init_member("getNewTextFormat", new builtin_function(textfield_getNewTextFormat));
	o.init_member("getNewTextFormat", new builtin_function(textfield_getNewTextFormat));
	o.init_member("getDepth", new builtin_function(textfield_getDepth));
	o.init_member("removeTextField", new builtin_function(textfield_removeTextField));
	o.init_member("replaceSel", new builtin_function(textfield_replaceSel));
	if ( target_version  < 7 ) return;

	// SWF7 or higher
	o.init_member("replaceText", new builtin_function(textfield_replaceText));
	if ( target_version  < 8 ) return;

}

static void
attachTextFieldStaticMembers(as_object& o)
{
	int target_version = o.getVM().getSWFVersion();

	// SWF5 or higher
	if ( target_version  < 6 ) return;

	// SWF6 or higher
	o.init_member("getFontList", new builtin_function(textfield_getFontList));
	if ( target_version  < 7 ) return;

	// SWF7 or higher
	if ( target_version  < 8 ) return;

}

static as_object*
getTextFieldInterface()
{
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachTextFieldInterface(*proto);
		proto->init_member("constructor", new builtin_function(textfield_ctor)); 
	}
	return proto.get();
}

//
// edit_text_character class
//

edit_text_character::edit_text_character(character* parent,
		edit_text_character_def* def, int id)
	:
	character(parent, id),
	m_def(def),
	_font(0),
	m_has_focus(false),
	m_cursor(0u),
	m_xcursor(0.0f),
	m_ycursor(0.0f),
	_text_variable_registered(false),
	_variable_name(m_def->get_variable_name())
{
	assert(parent);
	assert(m_def);

	set_prototype(getTextFieldInterface());

	// WARNING! remember to set the font *before* setting text value!
	set_font( m_def->get_font() );

	// set default text *before* calling registerTextVariable
	// (if the textvariable already exist and has a value
	//  the text will be replaced with it)
	set_text_value(m_def->get_default_text().c_str());

	m_dummy_style.push_back(fill_style());

	registerTextVariable();

	reset_bounding_box(0, 0);
}

edit_text_character::~edit_text_character()
{
	// TODO: unregisterTextVariable() ?
	on_event(event_id::KILLFOCUS);
}

void
edit_text_character::show_cursor()
{
	uint16_t x = static_cast<uint16_t>(m_xcursor);
	uint16_t y = static_cast<uint16_t>(m_ycursor);
	uint16_t h = m_def->get_font_height();

	int16_t box[4];
	box[0] = x;
	box[1] = y;
	box[2] = x;
	box[3] = y + h;
	
	render::draw_line_strip(box, 2, rgba(0,0,0,255));	// draw line
}

void
edit_text_character::display()
{
//		GNASH_REPORT_FUNCTION;

	registerTextVariable();

	rect def_bounds = m_def->get_bounds();

	if (m_def->has_border())
	{
		matrix	mat = get_world_matrix();
		
		// @@ hm, should we apply the color xform?  It seems logical; need to test.
		// cxform	cx = get_world_cxform();

		// Show white background + black bounding box.
		render::set_matrix(mat);
		//mat.print();

		point	coords[4];
		
		coords[0] = def_bounds.get_corner(0);
		coords[1] = def_bounds.get_corner(1);
		coords[2] = def_bounds.get_corner(2);
		coords[3] = def_bounds.get_corner(3);
		
		render::draw_poly(&coords[0], 4, rgba(255,255,255,255), rgba(0,0,0,255));
		
		
		// removed by Udo:
		/*
		coords[0] = def_bounds.get_corner(0);
		coords[1] = def_bounds.get_corner(1);
		coords[2] = def_bounds.get_corner(3);
		coords[3] = def_bounds.get_corner(2);

		int16_t	icoords[18] = 
		{
			// strip (fill in)
			(int16_t) coords[0].m_x, (int16_t) coords[0].m_y,
			(int16_t) coords[1].m_x, (int16_t) coords[1].m_y,
			(int16_t) coords[2].m_x, (int16_t) coords[2].m_y,
			(int16_t) coords[3].m_x, (int16_t) coords[3].m_y,

			// outline
			(int16_t) coords[0].m_x, (int16_t) coords[0].m_y,
			(int16_t) coords[1].m_x, (int16_t) coords[1].m_y,
			(int16_t) coords[3].m_x, (int16_t) coords[3].m_y,
			(int16_t) coords[2].m_x, (int16_t) coords[2].m_y,
			(int16_t) coords[0].m_x, (int16_t) coords[0].m_y,
		};
		
		render::fill_style_color(0, rgba(255, 255, 255, 255));
		render::draw_mesh_strip(&icoords[0], 4);

		render::line_style_color(rgba(0,0,0,255));
		render::draw_line_strip(&icoords[8], 5);
		*/
	}

	// Draw our actual text.
	// Using a matrix to translate to def bounds seems an hack to me.
	// A cleaner implementation is likely correctly setting the
	// m_x_offset and m_y_offset memebers in glyph records.
	// Anyway, see bug #17954 for a testcase.
	matrix m;

	if ( ! def_bounds.is_null() && ! def_bounds.is_world() )
	{
		m.concatenate_translation(def_bounds.get_x_min(), def_bounds.get_y_min());
	}
	
	
	display_glyph_records(m, this, m_text_glyph_records,
			      m_def->get_root_def());

	if (m_has_focus)
	{
		show_cursor();
	}

	clear_invalidated();
	do_display_callback();
}


void
edit_text_character::add_invalidated_bounds(InvalidatedRanges& ranges, 
	bool force)
{
	if (!force && !m_invalidated) return; // no need to redraw
    
	ranges.add(m_old_invalidated_ranges);

	rect bounds;	

	bounds.expand_to_transformed_rect(get_world_matrix(),
    				       m_def->get_bound());

	ranges.add(bounds.getRange());            
}

bool
edit_text_character::on_event(const event_id& id)
{
	if (m_def->get_readonly() == true)
	{
		return false;
	}

	switch (id.m_id)
	{
		case event_id::SETFOCUS:
		{
			if (m_has_focus == false)
			{
#ifdef NEW_KEY_LISTENER_LIST_DESIGN
				_vm.getRoot().add_key_listener(KeyListener(this));
#else
				_vm.getRoot().add_key_listener(this);
#endif
				m_has_focus = true;
				m_cursor = _text.size();
				format_text();
			}
			break;
		}

		case event_id::KILLFOCUS:
		{
			if (m_has_focus == true)
			{
				movie_root& root = _vm.getRoot();
				root.set_active_entity(NULL);
#ifdef NEW_KEY_LISTENER_LIST_DESIGN
				root.remove_key_listener(KeyListener(this));
#else
				root.remove_key_listener(this);
#endif
				m_has_focus = false;
				format_text();
			}
			break;
		}

		case event_id::KEY_PRESS:
		{
			std::string s(_text);
			std::string c;
			c = (char) id.m_key_code;

			// may be _text is changed in ActionScript
			m_cursor = imin(m_cursor, _text.size());

			switch (c[0])
			{
				case key::BACKSPACE:
					if (m_cursor > 0)
					{
						s.erase(m_cursor - 1, 1);
						m_cursor--;
						set_text_value(s.c_str());
					}
					break;

				case key::DELETEKEY:
					if (s.size() > m_cursor)
					{
						s.erase(m_cursor, 1);
						set_text_value(s.c_str());
					}
					break;

				case key::INSERT:		// TODO
					break;

				case key::HOME:
				case key::PGUP:
				case key::UP:
					m_cursor = 0;
					format_text();
					break;

				case key::END:
				case key::PGDN:
				case key::DOWN:
					m_cursor = _text.size();
					format_text();
					break;

				case key::LEFT:
					m_cursor = m_cursor > 0 ? m_cursor - 1 : 0;
					format_text();
					break;

				case key::RIGHT:
					m_cursor = m_cursor < _text.size() ? m_cursor + 1 : _text.size();
					format_text();
					break;

				default:
				{
					s.insert(m_cursor, c);
					m_cursor++;
					set_text_value(s.c_str());
					break;
				}
			}
		}

		default:
			return false;
	}
	return true;
}

character*
edit_text_character::get_topmost_mouse_entity(float x, float y)
{
	//log_msg("get_topmost_mouse_entity called on edit_text_character %p, labeled '%s'", (void*)this, get_text_value().c_str());

	if (get_visible() == false)
	{
		return NULL;
	}
	
	// shouldn't this be !can_handle_mouse_event() instead ?
	if (m_def->get_no_select())
	{
		// not selectable, so don't catch mouse events!
		return NULL;
	}

	matrix	m = get_matrix();
		
	point	p;
	m.transform_by_inverse(&p, point(x, y));

	const rect& def_bounds = m_def->get_bounds();
	if (def_bounds.point_test(p.m_x, p.m_y))
	{
		return this;
	}
	return NULL;
}

void
edit_text_character::set_text_value(const char* new_text_cstr)
{
	std::string new_text;
	if ( new_text_cstr ) new_text = new_text_cstr;

	if (_text == new_text)
	{
		return;
	}

	set_invalidated();

	_text = new_text;
	if (m_def->get_max_length() > 0
	    && _text.length() > m_def->get_max_length() )
	{
		_text.resize(m_def->get_max_length());
	}

	format_text();

	//log_msg(_("Text set to %s"), new_text.c_str());

}

std::string
edit_text_character::get_text_value() const
{
	// we need the const_cast here because registerTextVariable
	// *might* change our text value, calling the non-const
	// set_text_value().
	// This happens if the TextVariable has not been already registered
	// and during registration comes out to name an existing variable
	// with a pre-existing value.
	const_cast<edit_text_character*>(this)->registerTextVariable();

	return _text;
}

void
edit_text_character::set_member(const std::string& name,
		const as_value& val)
{
	//log_msg("edit_text_character.set_member(%s, %s)", name.c_str(), val.to_string());

	// FIXME: Turn all standard members into getter/setter properties
	//        of the TextField class. See attachTextFieldInterface()
	// @@ TODO need to inherit basic stuff like _x, _y, _xscale, _yscale etc ?


	as_standard_member	std_member = get_standard_member(name);
	switch (std_member)
	{
	default:
	case M_INVALID_MEMBER:
		break;
	case M_TEXT:
		//if (name == "text")
	{
		int version = get_parent()->get_movie_definition()->get_version();
		set_text_value(val.to_string_versioned(version).c_str());
		return;
	}
	case M_X:
		//else if (name == "_x")
	{
		matrix	m = get_matrix();
		m.m_[0][2] = infinite_to_fzero(PIXELS_TO_TWIPS(val.to_number()));	
		set_matrix(m);

		// m_accept_anim_moves = false;
		
		return;
	}
	case M_Y:
		//else if (name == "_y")
	{
		matrix	m = get_matrix();
		m.m_[1][2] = infinite_to_fzero(PIXELS_TO_TWIPS(val.to_number()));
		set_matrix(m);

		// m_accept_anim_moves = false;
		
		return;
	}
	case M_VISIBLE:
		//else if (name == "_visible")
	{
		set_visible(val.to_bool());
		return;
	}
	case M_ALPHA:
		//else if (name == "_alpha")
	{
		// @@ TODO this should be generic to class character!
		// Arg is in percent.
		cxform	cx = get_cxform();
		cx.m_[3][0] = fclamp(infinite_to_fzero(val.to_number()) / 100.f, 0, 1);
		set_cxform(cx);
		return;
	}
	case M_TEXTCOLOR:
		//else if (name == "textColor")
	{	
		// The arg is 0xRRGGBB format.
		uint32_t	rgb = (uint32_t) val.to_number();

		cxform	cx = get_cxform();
		cx.m_[0][0] = fclamp(((rgb >> 16) & 255) / 255.0f, 0, 1);
		cx.m_[1][0] = fclamp(((rgb >>  8) & 255) / 255.0f, 0, 1);
		cx.m_[2][0] = fclamp(((rgb      ) & 255) / 255.0f, 0, 1);
		set_cxform(cx);

		return;
	}
	// @@ TODO see TextField members in Flash MX docs
	}	// end switch

	set_member_default(name, val);
}

bool
edit_text_character::get_member(const std::string& name, as_value* val)
{
	//log_msg("edit_text_character.get_member(%s)", name.c_str());

	// FIXME: Turn all standard members into getter/setter properties
	//        of the TextField class. See attachTextFieldInterface()
	
	as_standard_member std_member = get_standard_member(name);
	switch (std_member)
	{
	default:
	case M_INVALID_MEMBER:
		break;
	case M_TEXT:
		//if (name == "text")
	{
		val->set_string(get_text_value());
		return true;
	}
	case M_VISIBLE:
		//else if (name == "_visible")
	{
		val->set_bool(get_visible());
		return true;
	}
	case M_ALPHA:
		//else if (name == "_alpha")
	{
		// @@ TODO this should be generic to class character!
		const cxform&	cx = get_cxform();
		val->set_double(cx.m_[3][0] * 100.f);
		return true;
	}
	case M_TEXTCOLOR:
		//else if (name == "textColor")
	{
		// Return color in 0xRRGGBB format
		const cxform&	cx = get_cxform();
		int	r = iclamp(int(cx.m_[0][0] * 255), 0, 255);
		int	g = iclamp(int(cx.m_[0][0] * 255), 0, 255);
		int	b = iclamp(int(cx.m_[0][0] * 255), 0, 255);
		val->set_int((r << 16) + (g << 8) + b);
		return true;
	}
	case M_X:
		//else if (name == "_x")
	{
		matrix	m = get_matrix();	// @@ get_world_matrix()???
		val->set_double(TWIPS_TO_PIXELS(m.m_[0][2]));
		return true;
	}
	case M_Y:
		//else if (name == "_y")
	{
		matrix	m = get_matrix();	// @@ get_world_matrix()???
		val->set_double(TWIPS_TO_PIXELS(m.m_[1][2]));
		return true;
	}
	case M_WIDTH: // _width
	{
		val->set_double(TWIPS_TO_PIXELS(get_width()));
		return true;
	}
	case M_HEIGHT: // _height
	{
		val->set_double(TWIPS_TO_PIXELS(get_height()));
		return true;
	}
	case M_TEXTWIDTH:
		//else if (name == "textWidth")
	{
		// Return the width, in pixels, of the text as laid out.
		// (I.e. the actual text content, not our defined
		// bounding box.)
		//
		// In local coords.  Verified against Macromedia Flash.
		val->set_double(TWIPS_TO_PIXELS(m_text_bounding_box.width()));

		return true;
	}
	}	// end switch

	return get_member_default(name, val);
}
	

float
edit_text_character::align_line(
		edit_text_character_def::alignment align,
		int last_line_start_record, float x)
{
	//GNASH_REPORT_FUNCTION;
	assert(m_def);

	float	extra_space = (m_def->width() -
			m_def->get_right_margin()) - x - PADDING_TWIPS;

	//assert(extra_space >= 0.0f);
	if (extra_space <= 0.0f)
	{
		log_debug(_("TextField text doesn't fit in its boundaries: "
			    "width %g, margin %d - nothing to align"),
			    m_def->width(), m_def->get_right_margin());
		return 0.0f;
	}

	float	shift_right = 0.0f;

	if (align == edit_text_character_def::ALIGN_LEFT)
	{
		// Nothing to do; already aligned left.
		return 0.0f;
	}
	else if (align == edit_text_character_def::ALIGN_CENTER)
	{
		// Distribute the space evenly on both sides.
		shift_right = extra_space / 2;
	}
	else if (align == edit_text_character_def::ALIGN_RIGHT)
	{
		// Shift all the way to the right.
		shift_right = extra_space;
	}

	// Shift the beginnings of the records on this line.
	for (unsigned int i = last_line_start_record; i < m_text_glyph_records.size(); i++)
	{
		text_glyph_record&	rec = m_text_glyph_records[i];

		if (rec.m_style.m_has_x_offset)
		{
			rec.m_style.m_x_offset += shift_right;
		}
	}
	return shift_right;
}

const font*
edit_text_character::set_font(const font* newfont)
{
	const font* oldfont = _font;
	_font = newfont; // @@ should I add_ref() ?
	return oldfont;  // @@ should I drop_ref() ?
}

void
edit_text_character::format_text()
{
	m_text_glyph_records.resize(0);

	// nothing more to do if text is empty
	if ( _text.empty() ) return;

	// FIXME: I don't think we should query the definition
	// to find the appropriate font to use, as ActionScript
	// code should be able to change the font of a TextField
	//
	if (_font == NULL)
	{
		log_error(_("No font for edit_text_character! [%s:%d]"),
			__FILE__, __LINE__);
		return;
	}

#if 0 // device fonts has an unknown (unless we scan them) number of glyphs
	// @@ mostly for debugging
	// Font substitution -- if the font has no
	// glyphs, try some other defined font!
	if (_font->get_glyph_count() == 0)
	{

		// Find a better font.
		const font*	newfont = _font;
		for (int i = 0, n = fontlib::get_font_count(); i < n; i++)
		{
			font*	f = fontlib::get_font(i);
			assert(f);

			if (f->get_glyph_count() > 0)
			{
				// This one looks good.
				newfont = f;
				break;
			}
		}

		if (_font != newfont)
		{
			log_error(_("substituting font!  font '%s' has "
				  "no glyphs, using font '%s'"),
				  fontlib::get_font_name(_font),
				  fontlib::get_font_name(newfont)
			);

			_font = newfont;
		}
		else
		{
			log_error(_("Current font has no glyphs and I couldn't"
				  " find another font with glyphs... :("));
		}

	}
#endif


	float	scale = m_def->get_font_height() / 1024.0f;	// the EM square is 1024 x 1024

	text_glyph_record	rec;	// one to work on
	rec.m_style.m_font = _font;
	rec.m_style.m_color = m_def->get_text_color();
	rec.m_style.m_x_offset = PADDING_TWIPS + std::max(0, m_def->get_left_margin() + m_def->get_indent());
	rec.m_style.m_y_offset = PADDING_TWIPS + m_def->get_font_height()
		+ (_font->get_leading() - _font->get_descent()) * scale;
	rec.m_style.m_text_height = m_def->get_font_height();
	rec.m_style.m_has_x_offset = true;
	rec.m_style.m_has_y_offset = true;

	float	x = rec.m_style.m_x_offset;
	float	y = rec.m_style.m_y_offset;
	

	// Start the bbox at the upper-left corner of the first glyph.
	reset_bounding_box(x, y - _font->get_descent() * scale + m_def->get_font_height());

	float	leading = m_def->get_leading();
	leading += _font->get_leading() * scale;

	int	last_code = -1;
	int	last_space_glyph = -1;
	int	last_line_start_record = 0;

	unsigned int character_idx = 0;
	m_xcursor = x;
	m_ycursor = y;

	assert(! _text.empty() );
	const char*	text = &_text[0]; 
	while (uint32_t code = utf8::decode_next_unicode_character(&text))
	{
// @@ try to truncate overflow text??
#if 0
		if (y + _font->get_descent() * scale > m_def->height())
		{
			// Text goes below the bottom of our bounding box.
			rec.m_glyphs.resize(0);
			break;
		}
#endif // 0

		//uint16_t	code = m_text[j];

		x += _font->get_kerning_adjustment(last_code, (int) code) * scale;
		last_code = static_cast<int>(code);

		// Expand the bounding-box to the lower-right corner of each glyph as
		// we generate it.
		m_text_bounding_box.expand_to_point(x, y + _font->get_descent() * scale);

		if (code == 13 || code == 10)
		{
			// newline.

			// Frigging Flash seems to use '\r' (13) as its
			// default newline character.  If we get DOS-style \r\n
			// sequences, it'll show up as double newlines, so maybe we
			// need to detect \r\n and treat it as one newline.

			// Close out this stretch of glyphs.
			m_text_glyph_records.push_back(rec);
			align_line(m_def->get_alignment(), last_line_start_record, x);

			// new paragraphs get the indent.
			x = std::max(0, m_def->get_left_margin() + m_def->get_indent()) + 
			  PADDING_TWIPS;
			y += m_def->get_font_height() + leading;

			// Start a new record on the next line.
			rec.m_glyphs.resize(0);
			rec.m_style.m_font = _font;
			rec.m_style.m_color = m_def->get_text_color();
			rec.m_style.m_x_offset = x;
			rec.m_style.m_y_offset = y;
			rec.m_style.m_text_height = m_def->get_font_height();
			rec.m_style.m_has_x_offset = true;
			rec.m_style.m_has_y_offset = true;

			last_space_glyph = -1;
			last_line_start_record = m_text_glyph_records.size();

			continue;
		}

		if (code == 8)
		{
			// backspace (ASCII BS).

			// This is a limited hack to enable overstrike effects.
			// It backs the cursor up by one character and then continues
			// the layout.  E.g. you can use this to display an underline
			// cursor inside a simulated text-entry box.
			//
			// ActionScript understands the '\b' escape sequence
			// for inserting a BS character.
			//
			// ONLY WORKS FOR BACKSPACING OVER ONE CHARACTER, WON'T BS
			// OVER NEWLINES, ETC.

			if (rec.m_glyphs.size() > 0)
			{
				// Peek at the previous glyph, and zero out its advance
				// value, so the next char overwrites it.
				float	advance = rec.m_glyphs.back().m_glyph_advance;
				x -= advance;	// maintain formatting
				rec.m_glyphs.back().m_glyph_advance = 0;	// do the BS effect
			}
			continue;
		}

		if (code == 9) // tab (ASCII HT)
		{
			int index = _font->get_glyph_index(32); // ascii SPACE
			if ( index == -1 )
			{
				IF_VERBOSE_MALFORMED_SWF (
				  log_error(_("%s -- missing glyph for space char (needed for TAB). "
					    " Make sure character shapes for font %s are being exported "
					    "into your SWF file."),
					    __PRETTY_FUNCTION__,
					    _font->get_name());
				);
			}
			else
			{
				text_glyph_record::glyph_entry	ge;
				ge.m_glyph_index = index;
				ge.m_glyph_advance = scale * _font->get_advance(index);

				const int tabstop=8;
				rec.m_glyphs.insert(rec.m_glyphs.end(), tabstop, ge);
				x += ge.m_glyph_advance*tabstop;
			}
			goto after_x_advance;
		}

		// Remember where word breaks occur.
		if (code == 32)
		{
			last_space_glyph = rec.m_glyphs.size();
		}

		{ // need a sub-scope to avoid the 'goto' in TAB handling to cross
		  // initialization of the 'index' variable
		int index = _font->get_glyph_index((uint16_t) code);
		IF_VERBOSE_MALFORMED_SWF (
		    if (index == -1)
		    {
			    // error -- missing glyph!
			    
			    // Log an error, but don't log too many times.
			    static int	s_log_count = 0;
			    if (s_log_count < 10)
			    {
				    s_log_count++;
				    log_swferror(_("%s -- missing glyph for char %d. "
						" Make sure character shapes for font %s are being exported "
						"into your SWF file"),
						__PRETTY_FUNCTION__,
						code,
						_font->get_name()
				    );
			    }

			    // Drop through and use index == -1; this will display
			    // using the empty-box glyph
		    }
		);
		text_glyph_record::glyph_entry	ge;
		ge.m_glyph_index = index;
		ge.m_glyph_advance = scale * _font->get_advance(index);

		rec.m_glyphs.push_back(ge);

		x += ge.m_glyph_advance;
		}
		
after_x_advance:

		if (x >= m_def->width() - m_def->get_right_margin() - PADDING_TWIPS)
		{
			// Whoops, we just exceeded the box width. 
			// Do word-wrap if requested to do so.

			if ( ! m_def->do_word_wrap() )
			{
				// TODO: scan more glyphs till newline and continue
				bool newlinefound = false;
				while ( (code = utf8::decode_next_unicode_character(&text)) )
				{
					if (code == 13 || code == 10)
					{
						newlinefound = true;
						break;
					}
				}
				if ( ! newlinefound ) break;
			}

			// Insert newline.

			// Close out this stretch of glyphs.
			m_text_glyph_records.push_back(rec);
			float	previous_x = x;
			x = m_def->get_left_margin() + PADDING_TWIPS;
			y += m_def->get_font_height() + leading;

			// Start a new record on the next line.
			rec.m_glyphs.resize(0);
			rec.m_style.m_font = _font;
			rec.m_style.m_color = m_def->get_text_color();
			rec.m_style.m_x_offset = x;
			rec.m_style.m_y_offset = y;
			rec.m_style.m_text_height = m_def->get_font_height();
			rec.m_style.m_has_x_offset = true;
			rec.m_style.m_has_y_offset = true;
			
			text_glyph_record&	last_line = m_text_glyph_records.back();
			if (last_space_glyph == -1)
			{
				// Pull the previous glyph down onto the
				// new line.
				if (last_line.m_glyphs.size() > 0)
				{
					rec.m_glyphs.push_back(last_line.m_glyphs.back());
					x += last_line.m_glyphs.back().m_glyph_advance;
					previous_x -= last_line.m_glyphs.back().m_glyph_advance;
					last_line.m_glyphs.resize(last_line.m_glyphs.size() - 1);
				}
			}
			else
			{
				// Move the previous word down onto the next line.

				previous_x -= last_line.m_glyphs[last_space_glyph].m_glyph_advance;

				for (unsigned int i = last_space_glyph + 1; i < last_line.m_glyphs.size(); i++)
				{
					rec.m_glyphs.push_back(last_line.m_glyphs[i]);
					x += last_line.m_glyphs[i].m_glyph_advance;
					previous_x -= last_line.m_glyphs[i].m_glyph_advance;
				}
				last_line.m_glyphs.resize(last_space_glyph);
			}

			align_line(m_def->get_alignment(), last_line_start_record, previous_x);

			last_space_glyph = -1;
			last_line_start_record = m_text_glyph_records.size();
		}

		if (m_cursor > character_idx)
		{
			m_xcursor = x;
			m_ycursor = y;
		}
		character_idx++;

		// TODO: HTML markup
	}

	// Add this line to our output.
	m_text_glyph_records.push_back(rec);

	float extra_space = align_line(m_def->get_alignment(), last_line_start_record, x);

	m_xcursor += static_cast<int>(extra_space);
	m_ycursor -= m_def->get_font_height() + (_font->get_leading() - _font->get_descent()) * scale;
}

void
edit_text_character::registerTextVariable() 
{
//#define DEBUG_DYNTEXT_VARIABLES 1

	if ( _text_variable_registered )
	{
#ifdef DEBUG_DYNTEXT_VARIABLES
		log_msg(_("registerTextVariable() no-op call (already registered)"));
#endif
		return;
	}

	if ( _variable_name.empty() )
	{
#ifdef DEBUG_DYNTEXT_VARIABLES
		log_msg(_("string is empty, consider as registered"));
#endif
		_text_variable_registered=true;
		return;
	}

	std::string var_str = _variable_name;
	VM& vm = VM::get();
	if ( vm.getSWFVersion() < 7 )
	{
		boost::to_lower( var_str, vm.getLocale() );
	}

	const char* varname = var_str.c_str();

#ifdef DEBUG_DYNTEXT_VARIABLES
	log_msg(_("VariableName: %s"), var_str.c_str());
#endif

	as_environment& env = get_environment();

	character* target = env.get_target();
	assert(target); // is this correct ?

	// If the variable string contains a path, we extract
	// the appropriate target from it and update the variable
	// name
	std::string path, var;
	if ( as_environment::parse_path(varname, path, var) )
	{
#ifdef DEBUG_DYNTEXT_VARIABLES
		log_msg(_("Variable text Path: %s, Var: %s"), path.c_str(), var.c_str());
#endif
		// find target for the path component
		// we use our parent's environment for this
		target = env.find_target(path);

		// update varname (with path component stripped)
		varname = var.c_str();
	}

	if ( ! target )
	{
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("VariableName associated to text field refer to an unknown target (%s). It is possible that the character will be instantiated later in the SWF stream. Gnash will try to register again on next access."), path.c_str());
		);
		return;
	}

	assert(dynamic_cast<sprite_instance*>(target));
	sprite_instance* sprite = static_cast<sprite_instance*>(target);


	// check if the VariableName already has a value,
	// in that case update text value
	as_value val;
	if ( sprite->get_member(varname, &val) )
	{
#ifdef DEBUG_DYNTEXT_VARIABLES
		log_msg(_("target sprite (%p) does have a member named %s"), (void*)sprite, varname);
#endif
		set_text_value(val.to_string().c_str());
	}
#ifdef DEBUG_DYNTEXT_VARIABLES
	else
	{
		log_msg(_("target sprite (%p) does NOT have a member named %s (no problem, we'll add it)"), (void*)sprite, varname);
	}
#endif

	// add the textfield variable to the target sprite
	sprite->set_textfield_variable(varname, this);

	_text_variable_registered=true;
}

void
edit_text_character::set_variable_name(const std::string& newname)
{
	if ( newname != _variable_name )
	{
		_variable_name = newname;
		_text_variable_registered = false;
	}
}

void
textfield_class_init(as_object& global)
{
	// This is going to be the global TextField "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl=NULL;

	if ( cl == NULL )
	{
		cl=new builtin_function(&textfield_ctor, getTextFieldInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachTextFieldStaticMembers(*cl);
		     
	}

	// Register _global.MovieClip
	global.init_member("TextField", cl.get());
}

bool
edit_text_character::pointInShape(float x, float y) const
{
	matrix wm = get_world_matrix();
	point lp(x, y);
	wm.transform_by_inverse(lp);
	const rect& def_bounds = m_def->get_bounds();
	return def_bounds.point_test(lp.m_x, lp.m_y);
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

