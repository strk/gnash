// textformat.cpp:  ActionScript text formatting decorators, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

// $Id: textformat.cpp,v 1.29 2007/09/19 14:20:49 cmusick Exp $

#include "log.h"
#include "textformat.h"
#include "fn_call.h"
#include "builtin_function.h" // for getter/setter properties

namespace gnash {  

  text_format::text_format() :
      _underline(false),
      _bold(false),
      _italic(false),
      _bullet(false),
      _block_indent(-1),
      _color(0),
      _indent(-1),
      _leading(-1),
      _left_margin(-1),
      _right_margin(-1),
      _point_size(-1),
      _tab_stops(-1),
      _target(-1)
{
  //log_msg("%s:", __FUNCTION__);
}

text_format::~text_format()
{
  // don't need to clean up anything
}

// Copy one text_format object to another.
text_format *
text_format::operator = (text_format &format)
{
  GNASH_REPORT_FUNCTION;

  _underline = format._underline;
  _bold = format._bold;
  _italic = format._italic;
  _bullet = format._bullet;
  
  _align = format._align;
  _block_indent = format._block_indent;
  _color = format._color;
  _font = format._font;
  _indent = format._indent;
  _leading = format._leading;
  _left_margin = format._left_margin;
  _right_margin = format._right_margin;
  _point_size = format._point_size;
  _tab_stops = format._tab_stops;
  _target = format._target;
  _url = format._url;
  
  return this;
}

// In a paragraph, change the format of a range of characters.
void
text_format::setTextFormat (text_format& /*format*/)
{
  //GNASH_REPORT_FUNCTION;
}

void
text_format::setTextFormat (int /*index*/, text_format& /*format*/)
{
  //GNASH_REPORT_FUNCTION;
}

void
text_format::setTextFormat (int /*start*/, int /*end*/, text_format& /*format*/)
{
  //GNASH_REPORT_FUNCTION;
}

#if 0
text_format &
text_format::getTextFormat ()
{
  GNASH_REPORT_FUNCTION;
}

text_format &
text_format::getTextFormat (int index)
{
  GNASH_REPORT_FUNCTION;
}

text_format &
text_format::getTextFormat (int start, int end)
{
  GNASH_REPORT_FUNCTION;
}
#endif

as_value textformat_new(const fn_call& /* fn */)
{
  //GNASH_REPORT_FUNCTION;
  //log_msg(_("%s: args=%d"), __FUNCTION__, nargs);

  boost::intrusive_ptr<textformat_as_object> text_obj = new textformat_as_object;
  log_unimpl(_("Created New TextFormat object at %p.  Not fully implemented yet"), (void*)text_obj.get());
  
  // tulrich: this looks like it's inserting a method into our
  // caller's env.  setTextFormat is a method on TextField.  So here
  // we're hoping our caller is a text field... scary.
  //
  // TODO we should handle setTextFormat as a method on TextField,
  // instead of doing this.
  //fn.env().set_variable("setTextFormat", new builtin_function(textformat_setformat));
  
  return as_value(text_obj.get());
}


as_value textformat_setformat(const fn_call& fn)
{
  as_value	method;
  //log_msg(_("%s: args=%d at %p"), __FUNCTION__, nargs, this_ptr);

  boost::intrusive_ptr<textformat_as_object> ptr = ensureType<textformat_as_object>(fn.this_ptr);
  //double start = fn.arg(0).to_number();
  //double end = fn.arg(1).to_number();

  if ( fn.nargs < 3 )
  {
    IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("TextFormat.setFormat() needs at least 3 arguments - ...me thinks"));
    );
    return as_value();
  }

  boost::intrusive_ptr<textformat_as_object> obj = boost::dynamic_pointer_cast<textformat_as_object>(fn.arg(2).to_object());
  if ( ! obj )
  {
    IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Argument 3 given to TextFormat.setFormat() is not a TextFormat object - ... should it be?"));
    );
    return as_value();
  }
  assert(obj);

  //log_msg(_("Change from %f for %f characters for object at %p"), start, end, obj);

  // Check for the flags that could be set
  if (obj->get_member(as_object::PROP_UNDERLINE, &method)) {
    //log_msg(_("Underline exists and is set to %d"), method.to_bool());
    obj->obj.underlinedSet(method.to_bool());
  }
  
  if (obj->get_member(as_object::PROP_ITALIC, &method)) {
    //log_msg(_("Italic exists and is set to %d"), method.to_bool());
    obj->obj.italicedSet(method.to_bool());
  }
  
  if (obj->get_member(as_object::PROP_BOLD, &method)) {
    //log_msg(_("Bold exists and is set to %d"), method.to_bool());
    obj->obj.boldSet(method.to_bool());
  }
  
  if (obj->get_member(as_object::PROP_BULLET, &method)) {
    //log_msg(_("Bullet exists and is set to %d"), method.to_bool());
    obj->obj.bulletSet(method.to_bool());
  }

  if (obj->get_member(as_object::PROP_COLOR, &method)) {
    //log_msg(_("Color exists and is set to %f", method.to_number());
    obj->obj.colorSet((uint32_t)method.to_number());
  }

  if (obj->get_member(as_object::PROP_INDENT, &method)) {
    //log_msg(_("Indent exists and is set to %f"), method.to_number());
    obj->obj.indentSet(float(method.to_number()));
  }

  if (obj->get_member(as_object::PROP_ALIGN, &method)) {
    //log_msg(_("Align exists and is set to %s"), method.to_string());
    const char* align = method.to_string().c_str();
    if ( align ) obj->obj.alignSet(align);
  }

  if (obj->get_member(as_object::PROP_BLOCK_INDENT, &method)) {
    //log_msg(_("BlockIndent exists and is set to %f"), method.to_number());
    obj->obj.blockIndentSet(float(method.to_number()));
  }
  
  if (obj->get_member(as_object::PROP_LEADING, &method)) {
    //log_msg(_("Leading exists and is set to %f"), method.to_number());
    obj->obj.leadingSet(float(method.to_number()));
  }
  
  if (obj->get_member(as_object::PROP_LEFT_MARGIN, &method)) {
    //log_msg(_("LeftMargin exists and is set to %f"), method.to_number());
    obj->obj.leftMarginSet(float(method.to_number()));
  }
  
  if (obj->get_member(as_object::PROP_RIGHT_MARGIN, &method)) {
    //log_msg(_("RightMargin exists and is set to %f"), method.to_number());
    obj->obj.rightMarginSet(float(method.to_number()));
  }
  
  if (obj->get_member(as_object::PROP_SIZE, &method)) {
    //log_msg(_("Size exists and is set to %f"), method.to_number());
    obj->obj.sizeSet(float(method.to_number()));
  }
  
  //ptr->obj.setTextFormat(start, end, obj->obj);
  //result->set_bool(true);
  return as_value();
}
#if 0
  void
  textformat_getformat(gnash::as_value* result, gnash::as_object_interface* this_ptr, gnash::as_environment* env, int nargs, int first_arg)
{
  log_unimpl(_("%s: args=%d unfinished implementation"), __FUNCTION__, nargs);
  textformat_as_object*	ptr = (textformat_as_object*)this_ptr;
  assert(ptr);
  double start = env->bottom(first_arg).to_number();
  double end = env->bottom(first_arg-1).to_number();
  textformat_as_object *obj = (textformat_as_object *)env->bottom(first_arg-2).to_object();
  assert(obj);
    
  ptr->obj = ptr->obj.getTextFormat();
  result->set_bool(true);
}
#endif

} // end of gnash namespace
